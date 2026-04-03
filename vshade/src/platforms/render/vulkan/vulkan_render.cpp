#include "engine/platforms/render/vulkan/vulkan_render.h"

vshade::render::VulkanRender::VulkanRender(const API api, std::uint32_t const frames_in_flight) : Render(api, frames_in_flight)
{
    descriptor::VulkanDescriptorManager::create<descriptor::VulkanDescriptorManager>(frames_in_flight);

    VulkanInstance& vulkan_instance{RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance()};
    VkDevice const  vk_logical_device{RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice()};

    /* Create global descriptor layout */
    vulkan_global_scene_data_.vulkan_descriptor_set_layout_ = std::make_unique<descriptor::VulkanDescriptorSetLayout>(
        vulkan_instance, vk_logical_device,
        std::initializer_list<VkDescriptorSetLayoutBinding>{
            //-----------------------Camera-----------------------//
            {_CAMERA_BUFFER_BINDING_, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1U,
             VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE},
        });

    //-----------------------PDrawIndex-----------------------//
    vulkan_global_scene_data_.vk_push_constant_ranges_.emplace_back(VkPushConstantRange{VK_SHADER_STAGE_FRAGMENT_BIT, 0U, sizeof(std::uint32_t)});

    query_pools_.resize(frames_in_flight);
}

vshade::render::VulkanRender::~VulkanRender()
{
    VkDevice const vk_logical_device{RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice()};

    for (auto& frame : query_pools_)
    {
        for (auto [_, query] : frame)
        {
            vkDestroyQueryPool(vk_logical_device, query.first, nullptr);
        }
    }

    descriptor::VulkanDescriptorManager::destroy();
}

void vshade::render::VulkanRender::beginRender(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                               std::shared_ptr<RenderPipeline> const render_pipeline, bool const is_clear,
                                               std::uint32_t const clear_count)
{
    enqueCommand(
        [=](std::uint32_t const frame_index)
        {
            VulkanFrameBuffer& vulkan_frame_buffer{render_pipeline->getSpecification().frame_buffer->as<VulkanFrameBuffer>()};
            VkCommandBuffer    vk_command_buffer{
                render_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer};
            VkRenderingInfo const& vk_rendering_info{vulkan_frame_buffer.getVkRenderingInfo()};

            std::vector<VkViewport> viewports{vk_rendering_info.layerCount};
            std::vector<VkRect2D>   scissors{vk_rendering_info.layerCount};

            FrameBuffer::Specification const& frame_buffer_specification{vulkan_frame_buffer.getSpecification()};

            for (std::uint32_t i{0U}; i < vk_rendering_info.layerCount; ++i)
            {
                viewports[i].x      = 0.f;
                viewports[i].y      = 0.f;
                viewports[i].width  = static_cast<float>(frame_buffer_specification.width);
                viewports[i].height = static_cast<float>(frame_buffer_specification.height);

                viewports[i].minDepth = 0.f; // Take a look
                viewports[i].maxDepth = 1.f; // Take a look

                scissors[i].offset.x      = 0;
                scissors[i].offset.y      = 0;
                scissors[i].extent.width  = frame_buffer_specification.width;
                scissors[i].extent.height = frame_buffer_specification.height;
            }

            vkCmdBeginRendering(vk_command_buffer, &vk_rendering_info);

            if (is_clear)
            {
                static VkClearRect vk_clear_rect;

                vk_clear_rect.rect           = vk_rendering_info.renderArea;
                vk_clear_rect.baseArrayLayer = 0U;
                vk_clear_rect.layerCount     = (clear_count) ? clear_count : vk_rendering_info.layerCount;

                vkCmdClearAttachments(vk_command_buffer, vulkan_frame_buffer.getVkClearAttachments().size(),
                                      vulkan_frame_buffer.getVkClearAttachments().data(), 1U, &vk_clear_rect);
            }

            vkCmdSetViewport(vk_command_buffer, 0U, viewports.size(), viewports.data());
            vkCmdSetScissor(vk_command_buffer, 0U, scissors.size(), scissors.data());

            vulkan_global_scene_data_.bindings_.buffers[_CAMERA_BUFFER_BINDING_][0] =
                submitted_frame_data_.camera_buffer->as<VulkanUniformBuffer>().getVkDescriptorBufferInfo(frame_index);

            render_pipeline->bind(render_command_buffer, frame_index);
        });
}

void vshade::render::VulkanRender::endRender(std::shared_ptr<RenderCommandBuffer> const render_command_buffer)
{
    enqueCommand(
        [=](std::uint32_t const frame_index)
        {
            VkCommandBuffer vk_command_buffer{
                render_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer};
            vkCmdEndRendering(vk_command_buffer);
        });
}

void vshade::render::VulkanRender::beginTimestampRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::uint32_t const frame_index,
                                                    std::string const& name)
{
    VkDevice const  vk_logical_device{RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice()};
    VkCommandBuffer vk_command_buffer{render_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer};

    if (query_pools_.at(frame_index)[name].first == VK_NULL_HANDLE)
    {
        VkQueryPoolCreateInfo vk_query_pool_create_info{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
        vk_query_pool_create_info.pNext              = VK_NULL_HANDLE;
        vk_query_pool_create_info.flags              = 0U;
        vk_query_pool_create_info.queryType          = VK_QUERY_TYPE_TIMESTAMP;
        vk_query_pool_create_info.queryCount         = 2U;
        vk_query_pool_create_info.pipelineStatistics = 0U;

        VK_CHECK_RESULT(vkCreateQueryPool(vk_logical_device, &vk_query_pool_create_info, nullptr, &query_pools_.at(frame_index).at(name).first),
                        "Failed to create query pool");
    }

    vkCmdResetQueryPool(vk_command_buffer, query_pools_.at(frame_index).at(name).first, 0U, 2U);
    vkCmdWriteTimestamp(vk_command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, query_pools_.at(frame_index).at(name).first, 0U);
}

void vshade::render::VulkanRender::endTimestampRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::uint32_t const frame_index,
                                                  std::string const& name)
{
    assert(query_pools_.at(frame_index).at(name).first != VK_NULL_HANDLE && "Trying to end undefined render timestamp !");
    vkCmdWriteTimestamp(render_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer,
                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, query_pools_.at(frame_index).at(name).first, 1U);
}

void vshade::render::VulkanRender::setBarrier(std::shared_ptr<RenderCommandBuffer>   render_command_buffer,
                                              std::shared_ptr<Texture2D const> const texture, Pipeline::Stage const src_stage,
                                              Pipeline::Stage const dst_stage, Pipeline::Access const src_access, Pipeline::Access const dst_accces,
                                              std::uint32_t const mip)
{
    VulkanRender::instance().enqueCommand(
        [=](std::uint32_t const frame_index)
        {
            VkImageMemoryBarrier vk_image_memory_barier{
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            };
            vk_image_memory_barier.pNext               = VK_NULL_HANDLE;
            vk_image_memory_barier.srcAccessMask       = static_cast<VkAccessFlags>(src_access);
            vk_image_memory_barier.dstAccessMask       = static_cast<VkAccessFlags>(dst_accces);
            vk_image_memory_barier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            vk_image_memory_barier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            vk_image_memory_barier.oldLayout           = texture->getImage()->as<VulkanImage2D>().getVkLayout();
            vk_image_memory_barier.newLayout           = VK_IMAGE_LAYOUT_GENERAL;
            vk_image_memory_barier.image               = texture->getImage()->as<VulkanImage2D>().getVkImage();
            vk_image_memory_barier.subresourceRange    = {
                texture->as<VulkanTexture2D>().getImage()->as<VulkanImage2D>().getAspectFlags(), // aspectMask
                (mip) ? mip : 0U,                                                                // baseMipLevel
                (mip) ? 1U : texture->getImage()->getSpecification().mip_count,                  // levelCount
                0U,                                                                              // baseArrayLayer
                texture->getImage()->getSpecification().layers                                   // layerCount
            };

            vkCmdPipelineBarrier(
                render_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer, // commandBuffer
                static_cast<VkPipelineStageFlags>(src_stage),                                                                 // srcStageMask
                static_cast<VkPipelineStageFlags>(dst_stage),                                                                 // dstStageMask
                0U,                                                                                                           // dependencyFlags
                0U,                                                                                                           // memoryBarrierCount
                VK_NULL_HANDLE,                                                                                               // pMemoryBarriers
                0U,                       // bufferMemoryBarrierCount
                VK_NULL_HANDLE,           // pBufferMemoryBarriers
                1U,                       // imageMemoryBarrierCount
                &vk_image_memory_barier); // pImageMemoryBarriers
        });
}

void vshade::render::VulkanRender::setBarrier(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                              std::shared_ptr<StorageBuffer const> const storage_buffer, Pipeline::Stage const src_stage,
                                              Pipeline::Stage const dst_stage, Pipeline::Access const src_access, Pipeline::Access const dst_accces)
{
    enqueCommand(
        [=](std::uint32_t const frame_index)
        {
            VkBufferMemoryBarrier vk_buffer_memory_barier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
            vk_buffer_memory_barier.pNext               = VK_NULL_HANDLE;
            vk_buffer_memory_barier.srcAccessMask       = static_cast<VkAccessFlags>(src_access);
            vk_buffer_memory_barier.dstAccessMask       = static_cast<VkAccessFlags>(dst_accces);
            vk_buffer_memory_barier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            vk_buffer_memory_barier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            vk_buffer_memory_barier.buffer              = storage_buffer->as<VulkanStorageBuffer>().getVkDescriptorBufferInfo(frame_index).buffer;
            vk_buffer_memory_barier.offset              = storage_buffer->as<VulkanStorageBuffer>().getVkDescriptorBufferInfo(frame_index).offset;
            vk_buffer_memory_barier.size                = storage_buffer->as<VulkanStorageBuffer>().getVkDescriptorBufferInfo(frame_index).range;

            vkCmdPipelineBarrier(
                render_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer, // commandBuffer
                static_cast<VkPipelineStageFlags>(src_stage),                                                                 // srcStageMask
                static_cast<VkPipelineStageFlags>(dst_stage),                                                                 // dstStageMask
                0U,                                                                                                           // dependencyFlags
                0U,                                                                                                           // memoryBarrierCount
                VK_NULL_HANDLE,                                                                                               // pMemoryBarriers
                1U,                       // bufferMemoryBarrierCount
                &vk_buffer_memory_barier, // pBufferMemoryBarriers
                0U,                       // imageMemoryBarrierCount
                VK_NULL_HANDLE);          // pImageMemoryBarriers
        });
}

void vshade::render::VulkanRender::queryResults(std::uint32_t const frame_index)
{
    // In nanoseconds
    VkDevice const vk_logical_device{RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice()};
    double const   period{RenderContext::instance()
                            .as<VulkanRenderContext>()
                            .getLogicalDevice()
                            ->getPhysicalDevice()
                            ->getPhysicalDeviceProperties()
                            .limits.timestampPeriod};

    for (auto& [name, query] : query_pools_.at(frame_index))
    {
        std::uint64_t start{0U}, end{0U};
        vkGetQueryPoolResults(vk_logical_device, query.first, 0U, 1U, sizeof(std::uint64_t), &start, 0U, VK_QUERY_RESULT_64_BIT);
        vkGetQueryPoolResults(vk_logical_device, query.first, 1U, 1U, sizeof(std::uint64_t), &end, 0U, VK_QUERY_RESULT_64_BIT);
        query.second = static_cast<double>(end - start) * period;
    }
}

double vshade::render::VulkanRender::getQueryResult(std::string const& name, std::uint32_t const frame_index) const
{
    auto query = query_pools_.at(frame_index).find(name);
    if (query != query_pools_.at(frame_index).end())
    {
        return query->second.second;
    }
    return 0.0;
}

void vshade::render::VulkanRender::clearFrameBufferAttachments(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                                               std::shared_ptr<FrameBuffer> const         frame_buffer)
{
    enqueCommand([=](std::uint32_t const frame_index) { frame_buffer->clearAttachmentsRT(render_command_buffer, frame_index); });
}

double vshade::render::VulkanRender::getQueryResult(std::string const& name) const
{
    std::uint32_t const frame_index{getCurrentPrepareFrameIndex()};
    auto                query = query_pools_.at(frame_index).find(name);
    if (query != query_pools_.at(frame_index).end())
    {
        return query->second.second;
    }
    return 0.0;
}

void vshade::render::VulkanRender::drawRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                          std::shared_ptr<VertexBuffer const> const  vertex_buffer,
                                          std::shared_ptr<IndexBuffer const> const index_buffer, std::uint32_t const frame_index,
                                          std::uint32_t const instance_count)
{
    static constexpr std::uint32_t VERTEX_BINDING{0U};
    static constexpr std::uint32_t INDEX_BINDING{1U};

    // TODO Need to add size types as Vertex adn Index
    vertex_buffer->bind(render_command_buffer, frame_index, VERTEX_BINDING);

    if (index_buffer)
    {
        std::uint32_t const index_count{index_buffer->getSize() / _IndexDataSize_};
        index_buffer->bind(render_command_buffer, frame_index, INDEX_BINDING);
        vkCmdDrawIndexed(render_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer, index_count,
                         instance_count, 0U, 0U, 0U);
    }
    else
    {
        vkCmdDraw(render_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer,
                  vertex_buffer->getVerticesCount(), instance_count, 0U, 0U);
    }
}

void vshade::render::VulkanRender::draw(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                        std::shared_ptr<VertexBuffer const> const  vertex_buffer,
                                        std::shared_ptr<IndexBuffer const> const index_buffer, std::uint32_t const instance_count)
{
    enqueCommand([=](std::uint32_t const frame_index) { drawRT(render_command_buffer, vertex_buffer, index_buffer, frame_index, instance_count); });
}