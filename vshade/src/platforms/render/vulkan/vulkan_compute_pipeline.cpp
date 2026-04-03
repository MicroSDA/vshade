
#include "engine/platforms/render/vulkan/vulkan_compute_pipeline.h"
#include "engine/platforms/render/vulkan/vulkan_render.h"

vshade::render::VulkanComputePipeline::VulkanComputePipeline(RenderPipeline::Specification const& specification) : ComputePipeline(specification)
{
    invalidate();
}

vshade::render::VulkanComputePipeline::~VulkanComputePipeline()
{
    VulkanInstance& vulkan_instance{RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance()};
    VkDevice const  vk_logical_device{RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice()};

    RenderContext::instance().enqueDelete(
        [vk_pipeline_layout = vk_pipeline_layout_, vk_pipeline = vk_pipeline_, vk_logical_device, vulkan_instance](std::uint32_t const frame_index)
        {
            if (vk_pipeline_layout != VK_NULL_HANDLE)
            {
                vkDestroyPipelineLayout(vk_logical_device, vk_pipeline_layout, vulkan_instance.allocation_callbaks);
            }

            if (vk_pipeline != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(vk_logical_device, vk_pipeline, vulkan_instance.allocation_callbaks);
            }
        });
}

void vshade::render::VulkanComputePipeline::invalidate()
{
    VulkanInstance& vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const  vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();

    for (auto& descriptor_layout : descriptors_layouts_)
    {
        descriptor_layout =
            std::make_shared<descriptor::VulkanDescriptorSetLayout>(vulkan_instance, vk_logical_device, std::vector<VkDescriptorSetLayoutBinding>{});
    }

    std::vector<VkPushConstantRange> vk_push_constant_ranges;

    for (auto& [set, data] : specification_.shader->as<VulkanShader>().getReflectedData())
    {
        std::vector<VkDescriptorSetLayoutBinding> vk_descriptor_set_layout_bindings;

        if (set != static_cast<std::uint32_t>(Pipeline::Set::_GLOBAL_))
        {
            //------------------------------------------------------------------------
            // Descriptor Set Layout Bindings: Storage Buffers
            //------------------------------------------------------------------------
            for (auto& [name, buffer] : data.storage_buffers)
            {
                VkDescriptorSetLayoutBinding vk_descriptor_set_layout_binding;
                vk_descriptor_set_layout_binding.binding            = buffer.binding;
                vk_descriptor_set_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                vk_descriptor_set_layout_binding.descriptorCount    = 1U;
                vk_descriptor_set_layout_binding.stageFlags         = static_cast<VkShaderStageFlags>(buffer.stage);
                vk_descriptor_set_layout_binding.pImmutableSamplers = VK_NULL_HANDLE;

                vk_descriptor_set_layout_bindings.emplace_back(vk_descriptor_set_layout_binding);
            }
            //------------------------------------------------------------------------
            // Descriptor Set Layout Bindings: Uniform Buffers
            //------------------------------------------------------------------------
            for (auto& [name, buffer] : data.uniform_buffers)
            {
                VkDescriptorSetLayoutBinding vk_descriptor_set_layout_binding;
                vk_descriptor_set_layout_binding.binding            = buffer.binding;
                vk_descriptor_set_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                vk_descriptor_set_layout_binding.descriptorCount    = 1U;
                vk_descriptor_set_layout_binding.stageFlags         = static_cast<VkShaderStageFlags>(buffer.stage);
                vk_descriptor_set_layout_binding.pImmutableSamplers = VK_NULL_HANDLE;

                vk_descriptor_set_layout_bindings.emplace_back(vk_descriptor_set_layout_binding);
            }
            //------------------------------------------------------------------------
            // Descriptor Set Layout Bindings: Image Samplers
            //------------------------------------------------------------------------
            for (auto& [name, image] : data.image_samplers)
            {
                VkDescriptorSetLayoutBinding vk_descriptor_set_layout_binding;
                vk_descriptor_set_layout_binding.binding            = image.binding;
                vk_descriptor_set_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                vk_descriptor_set_layout_binding.descriptorCount    = image.array_size;
                vk_descriptor_set_layout_binding.stageFlags         = static_cast<VkShaderStageFlags>(image.stage);
                vk_descriptor_set_layout_binding.pImmutableSamplers = VK_NULL_HANDLE;

                vk_descriptor_set_layout_bindings.emplace_back(vk_descriptor_set_layout_binding);
            }
            //------------------------------------------------------------------------
            // Descriptor Set Layout Bindings: Image storage
            //------------------------------------------------------------------------
            for (auto& [name, image] : data.image_storage)
            {
                VkDescriptorSetLayoutBinding vk_descriptor_set_layout_binding;
                vk_descriptor_set_layout_binding.binding            = image.binding;
                vk_descriptor_set_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                vk_descriptor_set_layout_binding.descriptorCount    = image.array_size;
                vk_descriptor_set_layout_binding.stageFlags         = static_cast<VkShaderStageFlags>(image.stage);
                vk_descriptor_set_layout_binding.pImmutableSamplers = VK_NULL_HANDLE;

                vk_descriptor_set_layout_bindings.emplace_back(vk_descriptor_set_layout_binding);
            }

            descriptors_layouts_.at(static_cast<std::uint32_t>(set)) =
                std::make_shared<descriptor::VulkanDescriptorSetLayout>(vulkan_instance, vk_logical_device, vk_descriptor_set_layout_bindings);
        }
        //------------------------------------------------------------------------
        // Push Constants
        //------------------------------------------------------------------------
        for (auto& [name, constant] : data.push_constants)
        {
            VkPushConstantRange vk_push_constant_range;
            vk_push_constant_range.stageFlags = constant.stage;
            vk_push_constant_range.offset     = 0U;
            vk_push_constant_range.size       = constant.size;

            vk_push_constant_ranges.emplace_back(vk_push_constant_range);
        }
    }

    //  std::vector<VkPushConstantRange> vk_push_constant_ranges{Render::instance().as<VulkanRender>().globalPushConstantRanges()};
    for (std::size_t i{1U}; i < vk_push_constant_ranges.size(); i++)
    {
        vk_push_constant_ranges[i].offset = vk_push_constant_ranges[i - 1U].size;
    }

    std::vector<VkDescriptorSetLayout> vk_descriptor_set_layout{
        Render::instance().as<VulkanRender>().getGlobalDescriptorSetLayout()->getVkDescriptorSetLayout()};

    for (Pipeline::Set set{Pipeline::Set::_PER_INSTANCE_}; set < Pipeline::Set::_MAX_ENUM_; ++set)
    {
        if (descriptors_layouts_.at(static_cast<std::size_t>(set)))
        {
            vk_descriptor_set_layout.emplace_back(descriptors_layouts_.at(static_cast<std::size_t>(set))->getVkDescriptorSetLayout());
        }
    }

    //------------------------------------------------------------------------
    // Pipeline Layout Creation
    //------------------------------------------------------------------------
    VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    vk_pipeline_layout_create_info.pNext                  = VK_NULL_HANDLE;
    vk_pipeline_layout_create_info.flags                  = 0U;
    vk_pipeline_layout_create_info.setLayoutCount         = static_cast<std::uint32_t>(vk_descriptor_set_layout.size());
    vk_pipeline_layout_create_info.pSetLayouts            = vk_descriptor_set_layout.data();
    vk_pipeline_layout_create_info.pushConstantRangeCount = static_cast<std::uint32_t>(vk_push_constant_ranges.size());
    vk_pipeline_layout_create_info.pPushConstantRanges    = vk_push_constant_ranges.data();

    VK_CHECK_RESULT(
        vkCreatePipelineLayout(vk_logical_device, &vk_pipeline_layout_create_info, vulkan_instance.allocation_callbaks, &vk_pipeline_layout_),
        "Failed to create vulkan compute pipeline layout!");

    //------------------------------------------------------------------------
    // Pipeline Creation
    //------------------------------------------------------------------------
    VkComputePipelineCreateInfo vk_compute_pipeline_create_info{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    vk_compute_pipeline_create_info.pNext = VK_NULL_HANDLE;
    vk_compute_pipeline_create_info.flags = 0U;
    vk_compute_pipeline_create_info.stage =
        specification_.shader->as<VulkanShader>().getVkPipelineShaderStageCreateInfo()[0]; // Should be one stage, compute, so 0 index
    vk_compute_pipeline_create_info.layout             = vk_pipeline_layout_;
    vk_compute_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    vk_compute_pipeline_create_info.basePipelineIndex  = 0U;

    VK_CHECK_RESULT(vkCreateComputePipelines(vk_logical_device, VK_NULL_HANDLE, 1U, &vk_compute_pipeline_create_info,
                                             vulkan_instance.allocation_callbaks, &vk_pipeline_),
                    "Failed to create graphics pipeline!");
    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_PIPELINE>(vulkan_instance.instance, "Vulkan render pipeline", vk_logical_device, vk_pipeline_);

    VSHADE_CORE_DEBUG("Vulkan compute pipeline: '{}' has been compiled successful.", specification_.name);
}

void vshade::render::VulkanComputePipeline::bindDescriptorsSets(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                                                std::uint32_t const                        frame_index)
{
    VkCommandBuffer vk_render_command_buffer{
        render_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer};

    //------------------------------------------------------------------------
    // Create VkDescriptorSet array and fill Pipeline::Set::_GLOBAL_
    //------------------------------------------------------------------------
    std::array<VkDescriptorSet, static_cast<std::size_t>(Pipeline::Set::_MAX_ENUM_)> vk_descriptor_sets{
        VulkanRender::instance().as<VulkanRender>().getGlobalDescriptorSet(frame_index)->getVkDescriptorSet()};

    //------------------------------------------------------------------------
    // Iterate and fill other sets
    //------------------------------------------------------------------------
    for (Pipeline::Set set{Pipeline::Set::_PER_INSTANCE_}; set < Pipeline::Set::_MAX_ENUM_; ++set)
    {
        std::size_t const _set_{static_cast<std::size_t>(set)};

        if (auto descriptor_set_layout{descriptors_layouts_.at(static_cast<std::size_t>(_set_))})
        {
            descriptor::VulkanDescriptorBindings const& vulkan_descriptor_bindings{resources_.at(_set_)};
            vk_descriptor_sets.at(static_cast<std::size_t>(set)) =
                descriptor::VulkanDescriptorManager::instance()
                    .reciveDescriptor(*descriptor_set_layout, vulkan_descriptor_bindings, frame_index)
                    ->getVkDescriptorSet();
        }
    }

    //------------------------------------------------------------------------
    // Bind all descriptors
    //------------------------------------------------------------------------
    vkCmdBindDescriptorSets(vk_render_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, vk_pipeline_layout_,
                            static_cast<std::uint32_t>(Pipeline::Set::_GLOBAL_), static_cast<std::uint32_t>(Pipeline::Set::_MAX_ENUM_),
                            vk_descriptor_sets.data(), 0U, VK_NULL_HANDLE);
}

void vshade::render::VulkanComputePipeline::updateResources(std::shared_ptr<RenderCommandBuffer> const render_command_buffer)
{
    VulkanRender::instance().enqueCommand([=](std::uint32_t const frame_index) { bindDescriptorsSets(render_command_buffer, frame_index); });
}

void vshade::render::VulkanComputePipeline::bind(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::uint32_t const frame_index)
{
    uniform_offset_ = 0U;
    vkCmdBindPipeline(render_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer,
                      VK_PIPELINE_BIND_POINT_COMPUTE, vk_pipeline_);
}

void vshade::render::VulkanComputePipeline::dispatch(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                                     std::uint32_t const group_count_x, std::uint32_t const group_count_y,
                                                     std::uint32_t const group_count_z)
{
    VulkanRender::instance().enqueCommand(
        [=](std::uint32_t const frame_index)
        {
            vkCmdDispatch(render_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer, group_count_x,
                          group_count_y, group_count_z);
        });
}

void vshade::render::VulkanComputePipeline::setStorageBuffer(std::shared_ptr<StorageBuffer const> const storage_buffer, Pipeline::Set const set,
                                                             std::uint32_t const offset)
{
    VulkanStorageBuffer const& vulkan_storage_buffer{storage_buffer->as<VulkanStorageBuffer>()};

    std::uint32_t const frame_index{VulkanRender::instance().getCurrentPrepareFrameIndex()};

    if (vulkan_storage_buffer.getVkDescriptorBufferInfos().size() >= frame_index)
    {
        resources_.at(static_cast<std::size_t>(set)).buffers[storage_buffer->getBindingIndex()][0U] =
            vulkan_storage_buffer.getVkDescriptorBufferInfo(frame_index);
        resources_.at(static_cast<std::size_t>(set)).buffers[storage_buffer->getBindingIndex()][0U].offset += offset;
        resources_.at(static_cast<std::size_t>(set)).buffers[storage_buffer->getBindingIndex()][0U].range -= offset;
    }
    else
    {
        VSHADE_CORE_WARNING("Trying to set storage buffer at frame index = {0}, but blocks count = {1} !", frame_index,
                            vulkan_storage_buffer.getVkDescriptorBufferInfos().size());
    }
}

void vshade::render::VulkanComputePipeline::setUniformBuffer(std::shared_ptr<UniformBuffer const> const uniform_buffer, Pipeline::Set const set,
                                                             std::uint32_t const offset)
{
    VulkanUniformBuffer const& vulkan_uniform_buffer{uniform_buffer->as<VulkanUniformBuffer>()};

    std::uint32_t const frame_index{VulkanRender::instance().getCurrentPrepareFrameIndex()};

    if (vulkan_uniform_buffer.getVkDescriptorBufferInfos().size() >= frame_index)
    {
        resources_.at(static_cast<std::size_t>(set)).buffers[uniform_buffer->getBindingIndex()][0U] =
            vulkan_uniform_buffer.getVkDescriptorBufferInfo(frame_index);
        resources_.at(static_cast<std::size_t>(set)).buffers[uniform_buffer->getBindingIndex()][0U].offset += offset;
        resources_.at(static_cast<std::size_t>(set)).buffers[uniform_buffer->getBindingIndex()][0U].range -= offset;
    }
    else
    {
        VSHADE_CORE_WARNING("Trying to set uniform buffer at frame index = {0}, but blocks count = {1} !", frame_index,
                            vulkan_uniform_buffer.getVkDescriptorBufferInfos().size());
    }
}

void vshade::render::VulkanComputePipeline::setTexture(std::shared_ptr<Texture2D const> const texture, Pipeline::Set const set,
                                                       std::uint32_t const binding, std::size_t const array_index)
{
    if (texture != nullptr)
    {
        resources_.at(static_cast<std::size_t>(set)).images[binding][array_index] = texture->as<VulkanTexture2D>().getDescriptorImageInfo();
    }
    else
    {
        resources_.at(static_cast<std::size_t>(set)).images[binding][array_index] =
            VulkanRender::instance().getDefaultMaterial()->texture_albedo->as<VulkanTexture2D>().getDescriptorImageInfo();
    }
}

void vshade::render::VulkanComputePipeline::setUniform(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::size_t const size,
                                                       void const* data, Shader::Stage const shader_stage)
{
    VulkanRender::instance().enqueCommand([=](std::uint32_t const frame_index)
                                          { setUniformRT(render_command_buffer, size, data, shader_stage, frame_index); });
}

void vshade::render::VulkanComputePipeline::setUniformRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::size_t const size,
                                                         void const* data, Shader::Stage const shader_stage, std::uint32_t const frame_index)
{
    VkCommandBuffer vulkan_render_command_buffer{
        render_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer};

    vkCmdPushConstants(vulkan_render_command_buffer, vk_pipeline_layout_, VulkanShader::fromShaderStageToVkShaderType(shader_stage), uniform_offset_,
                       size, data);
    uniform_offset_ += size;
}

void vshade::render::VulkanComputePipeline::recompile()
{
    if (auto shader = Shader::create(specification_.shader->getSpecification(), true))
    {
        specification_.shader = shader;
        invalidate();
    }
}
