#include "engine/core/render/render.h"
#include <engine/platforms/render/vulkan/vulkan_render.h>

void vshade::render::Render::create(const API api, std::uint32_t const frames_in_flight)
{
    RenderContext::create(api);
    RenderContext::instance().intitialize();

    switch (api)
    {
    case render::API::_VULKAN_:
        utility::CRTPSingleton<Render>::create<VulkanRender>(api, frames_in_flight);
        break;
    default:
        VSHADE_CORE_ERROR("OpenGL API isn't supported yet!")
    }

    instance().initialize(frames_in_flight);
}

void vshade::render::Render::initialize(std::uint32_t const frames_in_flight)
{
    render_command_queues_.resize(frames_in_flight);

    for (std::shared_ptr<RenderCommandQueue>& queue : instance().render_command_queues_)
    {
        queue = RenderCommandQueue::create<RenderCommandQueue>();
        time_stamps_.emplace_back(std::make_shared<std::atomic<std::int64_t>>());
    }

    submitted_frame_data_.global_transform_buffer =
        StorageBuffer::create(BufferUsage::_CPU_GPU_, _TRANSFORM_BUFFER_BINDING_, sizeof(glm::mat4), frames_in_flight, 50U);
    submitted_frame_data_.global_materials_buffer =
        StorageBuffer::create(BufferUsage::_CPU_GPU_, _MATERIAL_BUFFER_BINDING_, MATERIAL_DATA_SIZE(1U), frames_in_flight, 50U);
    submitted_frame_data_.camera_buffer = UniformBuffer::create(BufferUsage::_CPU_GPU_, _CAMERA_BUFFER_BINDING_, _CameraDataSize_, frames_in_flight);

    render::Image dummy_image;
    dummy_image.generateAsDummyImage();

    default_material_                    = Material::create<Material>();
    default_material_->texture_albedo    = Texture2D::createExplicit(Image2D::create(dummy_image));
    default_material_->texture_diffuse   = default_material_->texture_albedo;
    default_material_->texture_specular  = default_material_->texture_albedo;
    default_material_->texture_roughness = default_material_->texture_albedo;
    default_material_->texture_normals   = default_material_->texture_albedo;
}

void vshade::render::Render::destroy()
{
    // instance().render_thread_->join();
    utility::CRTPSingleton<Render>::destroy();
    RenderContext::destroy();
}

vshade::render::Render::Render(const API api, std::uint32_t const frames_in_flight)
    : current_api_{api}
    , frames_in_flight_{frames_in_flight}
    , render_frame_lock_{render_mutex_, std::defer_lock}
    , prepare_frame_lock_{render_mutex_, std::defer_lock}
{
    for (std::size_t i{0U}; i < frames_in_flight; ++i)
    {
        frames_in_flight_queue_.push_front(i);
    }
    // Create buffers
}

std::uint32_t vshade::render::Render::beginPrepareNewFrame()
{
    prepare_frame_lock_.lock(); // Enter critical section

    // Wait in case we have no avalible frames to prepare
    render_condition_variable_.wait(prepare_frame_lock_, [&] { return !frames_in_flight_queue_.empty(); });
    // Set current frame index from the pool
    std::uint32_t const current_frame_index{frames_in_flight_queue_.back()};
    current_process_frame_index_.store(current_frame_index, std::memory_order_release);

    // Set current time stamp
    time_stamps_.at(current_frame_index)
        ->store(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count(),
                std::memory_order_release);
    // Remove it from the pool
    frames_in_flight_queue_.pop_back();

    prepare_frame_lock_.unlock(); // Exit critical section

    cleanAllPendingCommands(current_process_frame_index_.load(std::memory_order::memory_order_acquire));
    rendered_instances_    = 0U;
    global_texture_offset_ = 0U;
    submitted_frame_data_.sorted_submission_map.clear();
    submitted_frame_data_.pipeline_submission_instance_map.clear();

    // submitted_frame_data_.instance_geometry_render_buffers.clear(); // Clear all submited geometry TEMPORARY !!

    // TODO:
    // {
    //     for (auto& [_, drawable_matarials] : submitted_frame_data_.pipeline_submission_instance_map)
    //     {
    //         for (auto& [_, model_per_material] : drawable_matarials)
    //         {
    //             bool expired{false};

    //             for (auto const& time_stamp : time_stamps_)
    //             {
    //                 if (time_stamp->load(std::memory_order_acquire) == model_per_material.time_stamp)
    //                 {
    //                     expired = true;
    //                     break;
    //                 }
    //             }

    //             if (expired)
    //             {
    //                 VSHADE_CORE_WARNING("Expired !");
    //             }
    //         }
    //     }
    // }

    return current_frame_index;
}

void vshade::render::Render::endPrepareNewFrame()
{
    // If render thread doesn't have frame to draw and sleep
    if (current_render_frame_index_.load(std::memory_order_acquire) == std::numeric_limits<std::uint32_t>::max())
    {
        current_render_frame_index_.store(current_process_frame_index_.load(std::memory_order_acquire), std::memory_order_release);
    }
    else
    {
        prepare_frame_lock_.lock(); // Enter critical section
        {
            // Push frame back into pool
            frames_in_flight_queue_.push_front(current_process_frame_index_.load(std::memory_order_acquire));
        }
        prepare_frame_lock_.unlock(); // Exit critical section
    }
}

void vshade::render::Render::processRenderThread()
{
    if (render_thread_ == nullptr)
    {
        render_thread_ = std::make_shared<std::thread>(
            [&]
            {
                while (!is_quit_reqested_.load(std::memory_order_relaxed))
                {
                    std::uint32_t const frame_index{beginRenderFrame()};

                    executeAllPendingCommands(frame_index);

                    render::SwapChain::instance().beginFrame(frame_index);

                    if (swap_chain_resolve_texture_ != nullptr)
                    {
                        render::SwapChain::instance().resolveIntoSwapChain(swap_chain_resolve_texture_, frame_index);
                        swap_chain_resolve_texture_.reset();
                    }

                    render::SwapChain::instance().endFrame(frame_index);
                    queryResults(frame_index);
                    if (render::SwapChain::instance().present(std::chrono::nanoseconds::max(), frame_index))
                    {
                        endRenderFrame();
                        RenderContext::instance().deleteAllPendings(frame_index);
                    }
                }
            });
    }
}

std::uint32_t vshade::render::Render::beginRenderFrame()
{
    // Wait untill main tread set the avalible frame indiex
    while (current_render_frame_index_.load(std::memory_order_acquire) == std::numeric_limits<std::uint32_t>::max())
    {
        std::this_thread::yield();
    }

    return current_render_frame_index_.load(std::memory_order_acquire);
}

void vshade::render::Render::endRenderFrame()
{
    render_frame_lock_.lock(); // Enter critical section
    {
        frames_in_flight_queue_.push_front(current_render_frame_index_.load(std::memory_order_acquire));
        current_render_frame_index_.store(std::numeric_limits<std::uint32_t>::max(), std::memory_order_release);
    }
    render_frame_lock_.unlock(); // Exit critical section

    // Notify main thread that we are finished with render and it can continue(in case we have only one frame in time)
    render_condition_variable_.notify_all();
}

void vshade::render::Render::closeRenderThread()
{
    is_quit_reqested_.store(true, std::memory_order_relaxed);
    render_thread_->join();
}

void vshade::render::Render::resolveIntoSwapChain(std::shared_ptr<Texture2D const> const texture)
{
    swap_chain_resolve_texture_ = texture;
}

void vshade::render::Render::submit(std::shared_ptr<RenderPipeline const> const pipeline, std::shared_ptr<Drawable const> const instance,
                                    std::shared_ptr<Material const> const material, glm::mat4 const& transform, SubmitPolicy const policy,
                                    std::size_t const split_offset)
{
    if (pipeline->isActive())
    {
        std::int64_t const current_time_stamp{*time_stamps_.at(getCurrentPrepareFrameIndex())};
        std::size_t const  pipeline_hash{utils::hash(pipeline)};
        std::size_t const  instance_hash{utils::hash(instance)};
        std::size_t const  combined_hash{utils::hash(pipeline, instance, split_offset)};

        submitted_frame_data_.sorted_submission_map[combined_hash].transforms.emplace_back(transform);
        submitted_frame_data_.sorted_submission_map[combined_hash].materials.emplace_back((material) ? material : default_material_);
        submitted_frame_data_.sorted_submission_map[combined_hash].materials_render_data.emplace_back(
            (material) ? material->getRenderData() : default_material_->getRenderData());

        submitted_frame_data_.pipeline_submission_instance_map[pipeline_hash][instance_hash].materials.insert(material);
        submitted_frame_data_.pipeline_submission_instance_map[pipeline_hash][instance_hash].time_stamp = current_time_stamp;

        // If geometry has not been created yet
        if (submitted_frame_data_.instance_geometry_render_buffers.find(instance_hash) ==
            submitted_frame_data_.instance_geometry_render_buffers.end())
        {
            createInstanceGeometryBuffer(instance, policy);
        }
        else
        {
            if (policy == SubmitPolicy::_STREAM_)
            {
                setDataInstanceGeometryBuffer(instance, submitted_frame_data_.instance_geometry_render_buffers[instance_hash], policy, 0U);
            }
        }
    }
}

void vshade::render::Render::submit(std::shared_ptr<RenderPipeline const> const pipeline, std::shared_ptr<VertexBuffer const> const vertex_buffer,
                                    std::shared_ptr<IndexBuffer const> const index_buffer, std::shared_ptr<Material const> const material,
                                    glm::mat4 const& transform, std::size_t const split_offset)
{
    if (pipeline->isActive())
    {
        std::int64_t const current_time_stamp{*time_stamps_.at(getCurrentPrepareFrameIndex())};
        std::size_t const  pipeline_hash{utils::hash(pipeline)};
        std::size_t const  instance_hash{utils::hash(vertex_buffer)};
        std::size_t const  combined_hash{utils::hash(pipeline, vertex_buffer, split_offset)};

        submitted_frame_data_.sorted_submission_map[combined_hash].transforms.emplace_back(transform);

        submitted_frame_data_.sorted_submission_map[combined_hash].materials.emplace_back((material) ? material : default_material_);

        submitted_frame_data_.sorted_submission_map[combined_hash].materials_render_data.emplace_back(
            (material) ? material->getRenderData() : default_material_->getRenderData());

        submitted_frame_data_.pipeline_submission_instance_map[pipeline_hash][instance_hash].materials.emplace(material);
        submitted_frame_data_.pipeline_submission_instance_map[pipeline_hash][instance_hash].time_stamp = current_time_stamp;

        // If geometry has not been created yet
        if (submitted_frame_data_.instance_geometry_render_buffers.find(instance_hash) ==
            submitted_frame_data_.instance_geometry_render_buffers.end())
        {
            data::InstanceGeometryRenderBuffers& buffer{submitted_frame_data_.instance_geometry_render_buffers[instance_hash]};

            buffer.vertex_buffer = std::const_pointer_cast<VertexBuffer>(vertex_buffer);
            buffer.index_buffer  = std::const_pointer_cast<IndexBuffer>(index_buffer);
        }
    }
}

void vshade::render::Render::drawSubmitedInstance(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                                  std::shared_ptr<RenderPipeline> const pipeline, std::size_t const instance,
                                                  std::size_t const split_offset)
{
    std::size_t const combined_hash{utils::hash(pipeline, instance, split_offset)};

    auto sorted_data = submitted_frame_data_.sorted_submission_map.find(combined_hash);

    if (sorted_data != submitted_frame_data_.sorted_submission_map.end())
    {
        // Set render instance id
        setUniform<std::uint32_t, Shader::Stage::_FRAGMENT_>(render_command_buffer, pipeline, &rendered_instances_);
        updateResources(render_command_buffer, pipeline);

        enqueCommand([this, render_command_buffer_ = render_command_buffer, count = sorted_data->second.transforms.size(),
                      buffers = submitted_frame_data_.instance_geometry_render_buffers.at(instance)](std::uint32_t const frame_index)
                     { drawRT(render_command_buffer_, buffers.vertex_buffer, buffers.index_buffer, frame_index, count); });

        rendered_instances_ += sorted_data->second.transforms.size();
    }
}

bool vshade::render::Render::executeSubmittedRenderPipeline(std::shared_ptr<RenderPipeline> const      pipeline,
                                                            std::shared_ptr<RenderCommandBuffer> const render_comand_buffer, bool is_force_clear)
{
    auto search = submitted_frame_data_.pipeline_submission_instance_map.find(utils::hash(pipeline));

    if (pipeline->isActive() && search != submitted_frame_data_.pipeline_submission_instance_map.end())
    {
        pipeline->getProcessCallback()(pipeline, render_comand_buffer, search->second, submitted_frame_data_, current_process_frame_index_,
                                       is_force_clear);

        return true;
    }

    return false;
}

bool vshade::render::Render::executeSubmittedComputePipeline(std::shared_ptr<ComputePipeline> const     pipeline,
                                                             std::shared_ptr<RenderCommandBuffer> const render_comand_buffer)
{
    if (pipeline->isActive())
    {
        pipeline->getProcessCallback()(pipeline, render_comand_buffer, current_process_frame_index_);

        return true;
    }

    return false;
}

void vshade::render::Render::createInstanceGeometryBuffer(std::shared_ptr<Drawable const> const instance, SubmitPolicy const policy)
{
    if (instance)
    {
        data::InstanceGeometryRenderBuffers& buffer{submitted_frame_data_.instance_geometry_render_buffers[utils::hash(instance)]};

        auto const& full_geometry{instance->getFullGeometry()};

        static constexpr std::size_t lod{0U}; // Has to be changed, need to figure out how to set all levels into buffer !

        // At this moment all vertex buffers that were been created here have only one frame in flight ! SubmitPolicy Stream
        buffer.vertex_buffer =
            render::VertexBuffer::create((policy == SubmitPolicy::_DEFAULT_) ? render::BufferUsage::_GPU_ : render::BufferUsage::_CPU_GPU_,
                                         VERTICES_DATA_SIZE(1U), full_geometry[lod].vertices.size(), 1U, 0U);
        buffer.vertex_buffer->setData(VERTICES_DATA_SIZE(1U), full_geometry[lod].vertices.size(), full_geometry[lod].vertices.data(), 0U);

        buffer.index_buffer =
            render::IndexBuffer::create((policy == SubmitPolicy::_DEFAULT_) ? render::BufferUsage::_GPU_ : render::BufferUsage::_CPU_GPU_,
                                        INDICES_DATA_SIZE(full_geometry[lod].indices.size()), 0U, full_geometry[lod].indices.data());
    }
}

void vshade::render::Render::setDataInstanceGeometryBuffer(std::shared_ptr<Drawable const> const instance,
                                                           data::InstanceGeometryRenderBuffers& buffers, SubmitPolicy const policy,
                                                           std::uint32_t const offset)
{
    auto const&                  full_geometry{instance->getFullGeometry()};
    static constexpr std::size_t lod{0U}; // Has to be changed, need to figure out how to set all levels into buffer !

    std::size_t const vertex_buffer_size{VERTICES_DATA_SIZE(full_geometry[lod].vertices.size())};
    std::size_t const index_buffer_size{INDICES_DATA_SIZE(full_geometry[lod].indices.size())};

    if (vertex_buffer_size > buffers.vertex_buffer->getSize() || index_buffer_size > buffers.index_buffer->getSize())
    {
        createInstanceGeometryBuffer(instance, policy);
    }
    else
    {
        buffers.vertex_buffer->setData(VERTICES_DATA_SIZE(1U), full_geometry[lod].vertices.size(), full_geometry[lod].vertices.data(), 0U, offset);
        buffers.index_buffer->setData(INDICES_DATA_SIZE(full_geometry[lod].indices.size()), full_geometry[lod].indices.data(), offset);
    }
}

void vshade::render::Render::setSubmittedTransforms(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                                    std::shared_ptr<RenderPipeline> const pipeline, std::size_t const instance,
                                                    std::size_t const split_offset)
{
    std::size_t const combined_hash{utils::hash(pipeline, instance, split_offset)};

    auto const sorted_data = submitted_frame_data_.sorted_submission_map.find(combined_hash);

    if (sorted_data != submitted_frame_data_.sorted_submission_map.end())
    {
        pipeline->setStorageBuffer(submitted_frame_data_.global_transform_buffer, Pipeline::Set::_PER_INSTANCE_, sorted_data->second.tansform_offset);
    }
}

void vshade::render::Render::setSubmittedMaterials(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                                   std::shared_ptr<RenderPipeline> const pipeline, std::size_t const instance,
                                                   std::size_t const split_offset)
{
    std::size_t const combined_hash{utils::hash(pipeline, instance, split_offset)};

    auto const sorted_data = submitted_frame_data_.sorted_submission_map.find(combined_hash);

    if (sorted_data != submitted_frame_data_.sorted_submission_map.end())
    {
        std::size_t index_offset{global_texture_offset_};

        pipeline->setStorageBuffer(submitted_frame_data_.global_materials_buffer, Pipeline::Set::_PER_INSTANCE_, sorted_data->second.material_offset);

        for (std::size_t i{0U}; i < sorted_data->second.materials.size(); ++i)
        {
            std::shared_ptr<Material const> const material{sorted_data->second.materials.at(i)};

            pipeline->setTexture(material->texture_albedo, Pipeline::Set::_PER_INSTANCE_, _MATERIAL_TEXTURES_BINDING_,
                                 index_offset + _ALBEDO_TEXTURE_INDEX_);
            pipeline->setTexture(material->texture_diffuse, Pipeline::Set::_PER_INSTANCE_, _MATERIAL_TEXTURES_BINDING_,
                                 index_offset + _DIFFUSE_TEXTURE_INDEX_);
            pipeline->setTexture(material->texture_specular, Pipeline::Set::_PER_INSTANCE_, _MATERIAL_TEXTURES_BINDING_,
                                 index_offset + _SPECULAR_TEXTURE_INDEX_);
            pipeline->setTexture(material->texture_specular, Pipeline::Set::_PER_INSTANCE_, _MATERIAL_TEXTURES_BINDING_,
                                 index_offset + _NORMAL_MAP_TEXTURE_INDEX_);

            index_offset += 4U;
        }

        global_texture_offset_ = index_offset;
    }
}

void vshade::render::Render::beginScene(std::shared_ptr<Camera> const camera, std::shared_ptr<RenderCommandBuffer> const render_command_buffer)
{
    prepareBuffersOffsets();

    Camera::RenderData camera_render_data{camera->getRenderData()};

    // // Is thats makes sense ?
    // camera_render_data.near = (-camera_render_data.projection[3][2]);
    // camera_render_data.far  = (camera_render_data.projection[2][2]);

    // if (camera_render_data.near * camera_render_data.far < 0)
    // 	camera_render_data.far = -camera_render_data.far;

    submitted_frame_data_.camera_buffer->setData(_CameraDataSize_, &camera_render_data, getCurrentPrepareFrameIndex());

    enqueCommand(
        [=](std::uint32_t const frame_index)
        {
            if (!render_command_buffer->isInRecorderStage(frame_index))
            {
                render_command_buffer->begin(frame_index);
            }
        });
}

void vshade::render::Render::endScene(std::shared_ptr<RenderCommandBuffer> const render_command_buffer)
{
    enqueCommand(
        [=](std::uint32_t const frame_index)
        {
            if (render_command_buffer->isInRecorderStage(frame_index))
            {
                render_command_buffer->end(frame_index);
                render_command_buffer->submit(frame_index);
            }
        });
}

void vshade::render::Render::beginCompute(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                          std::shared_ptr<ComputePipeline> const     compute_pipeline)
{
    enqueCommand(
        [=](std::uint32_t const frame_index)
        {
            if (!render_command_buffer->isInRecorderStage(frame_index))
            {
                render_command_buffer->begin(frame_index);
            }

            compute_pipeline->bind(render_command_buffer, frame_index);
        });
}

void vshade::render::Render::endCompute(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                        std::shared_ptr<ComputePipeline> const compute_pipeline, bool const is_submit)
{
    enqueCommand(
        [=](std::uint32_t const frame_index)
        {
            if (is_submit && render_command_buffer->isInRecorderStage(frame_index))
            {
                render_command_buffer->end(frame_index);
                render_command_buffer->submit(frame_index);
            }
        });
}

std::uint32_t const vshade::render::Render::getCurrentPrepareFrameIndex() const
{
    return current_process_frame_index_.load(std::memory_order_acquire);
}

std::uint32_t const vshade::render::Render::getCurrentRenderFrameIndex() const
{
    return current_render_frame_index_.load(std::memory_order_acquire);
}

void vshade::render::Render::prepareBuffersOffsets()
{
    std::size_t count{0U};

    for (auto& [hash, instance] : submitted_frame_data_.sorted_submission_map)
    {
        // Set the transform and material offsets for each instance
        instance.tansform_offset = sizeof(glm::mat4) * count;
        instance.material_offset = MATERIAL_DATA_SIZE(count);

        for (std::size_t i{0U}; i < instance.transforms.size(); ++i)
        {
            // Increment the count of transforms and materials.
            ++count;
        }
    }

    // Resize the transform and materials buffers based on the number instances.
    submitted_frame_data_.global_transform_buffer->resize(sizeof(glm::mat4) * count);
    submitted_frame_data_.global_materials_buffer->resize(MATERIAL_DATA_SIZE(count));

    std::uint32_t const frame_index{getCurrentPrepareFrameIndex()};

    for (auto& [hash, instance] : submitted_frame_data_.sorted_submission_map)
    {
        // Set the transform and material data for each instance in the buffer
        submitted_frame_data_.global_transform_buffer->setData(sizeof(glm::mat4) * instance.transforms.size(), instance.transforms.data(),
                                                               frame_index, instance.tansform_offset);
        submitted_frame_data_.global_materials_buffer->setData(MATERIAL_DATA_SIZE(instance.materials_render_data.size()),
                                                               instance.materials_render_data.data(), frame_index, instance.material_offset);
    }
}

void vshade::render::Render::waitUntilRenderDone()
{
    while (current_render_frame_index_.load(std::memory_order_acquire) != std::numeric_limits<std::uint32_t>::max())
    {
        std::this_thread::yield();
    }
}

void vshade::render::Render::beginTimestamp(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::string const& name)
{
    enqueCommand([=](std::uint32_t const frame_index) { beginTimestampRT(render_command_buffer, frame_index, name); });
}

void vshade::render::Render::endTimestamp(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::string const& name)
{
    enqueCommand([=](std::uint32_t const frame_index) { endTimestampRT(render_command_buffer, frame_index, name); });
}

void vshade::render::Render::setUniformRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::shared_ptr<Pipeline> const pipeline,
                                          std::size_t const size, void const* data, Shader::Stage const shader_stage, std::uint32_t const frame_index)
{
    pipeline->setUniformRT(render_command_buffer, size, data, shader_stage, frame_index);
}

void vshade::render::Render::updateResources(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                             std::shared_ptr<Pipeline> const            pipeline)
{
    pipeline->updateResources(render_command_buffer);
}

void vshade::render::Render::dispatch(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                      std::shared_ptr<ComputePipeline> const pipeline, std::uint32_t const group_count_x,
                                      std::uint32_t const group_count_y, std::uint32_t const group_count_z)
{
    pipeline->updateResources(render_command_buffer);
    pipeline->dispatch(render_command_buffer, group_count_x, group_count_y, group_count_z);
}