#include "engine/platforms/render/vulkan/vulkan_render_pipeline.h"
#include "engine/platforms/render/vulkan/vulkan_render.h"

vshade::render::VulkanRenderPipeline::VulkanRenderPipeline(RenderPipeline::Specification const& specification) : RenderPipeline(specification)
{
    invalidate();
}

vshade::render::VulkanRenderPipeline::~VulkanRenderPipeline()
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

void vshade::render::VulkanRenderPipeline::invalidate()
{
    VulkanInstance& vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const  vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();

    if (specification_.frame_buffer == nullptr || specification_.shader == nullptr)
    {
        VSHADE_CORE_ERROR("Frame buffer is nullptr!")
    }

    for (auto& descriptor_layout : descriptors_layouts_)
    {
        descriptor_layout =
            std::make_shared<descriptor::VulkanDescriptorSetLayout>(vulkan_instance, vk_logical_device, std::vector<VkDescriptorSetLayoutBinding>{});
    }

    //------------------------------------------------------------------------
    // Add global push constant
    //------------------------------------------------------------------------
    std::vector<VkPushConstantRange> vk_push_constant_ranges{VulkanRender::instance().as<VulkanRender>().getGlobalVkPushConstantRanges()};

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

            //------------------------------------------------------------------------
            // We can have only one push constant per stage, vertex stage is free for
            // now, fragment is occupied by global push constant
            //------------------------------------------------------------------------
            if (std::find_if(vk_push_constant_ranges.begin(), vk_push_constant_ranges.end(), [=](VkPushConstantRange const& global)
                             { return (global.stageFlags & constant.stage) != 0U; }) == vk_push_constant_ranges.end())
            {
                vk_push_constant_ranges.emplace_back(vk_push_constant_range);
            }
        }
    }

    //------------------------------------------------------------------------
    // Pipeline Layout Creation
    //------------------------------------------------------------------------

    /* Alongside of all the State structs, we will need a VkPipelineLayout object for our pipeline.
    Unlike the other state structs, this one is an actual full Vulkan object, and needs to be created separately from the pipeline.
    Pipeline layouts contain the information about shader inputs of a given pipeline.
    It’s here where you would configure your push-constants and descriptor sets. */

    //  std::vector<VkPushConstantRange> vk_push_constant_ranges{Render::instance().as<VulkanRender>().globalPushConstantRanges()};

    std::map<VkShaderStageFlagBits, uint32_t> stages_offsets; // Map to hold stage offsets for push constants

    for (auto& push_constant : vk_push_constant_ranges)
    {
        VkShaderStageFlags stageFlags = push_constant.stageFlags;

        // Iterate through all possible shader stages
        for (VkShaderStageFlagBits stageBit = VK_SHADER_STAGE_VERTEX_BIT; stageBit <= VK_SHADER_STAGE_COMPUTE_BIT; // Why compute ?
             stageBit                       = static_cast<VkShaderStageFlagBits>(stageBit << 1))
        {
            if (stageFlags & stageBit)
            {
                // Find the offset for the current stage in the map
                auto stage = stages_offsets.find(stageBit);

                // If the stage is not found, add it to the map with an initial offset of 0
                if (stage == stages_offsets.end())
                {
                    stages_offsets[stageBit] = 0U;
                }
                else
                {
                    // Increment the offset for the current stage
                    stage->second += push_constant.size;
                }

                // Update the offset in the pushConstant structure
                push_constant.offset = stages_offsets[stageBit];
            }
        }
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
    // Pipeline Layout Create Info
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
        "Failed to create vulkan render pipeline layout!");

    //------------------------------------------------------------------------
    // Input Assembly State
    //------------------------------------------------------------------------

    /*  Contains the configuration for what kind of topology will be drawn.
        This is where you set it to draw triangles, lines, points, or others like triangle-list. */
    VkPipelineInputAssemblyStateCreateInfo vk_pipeline_input_assembly_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    vk_pipeline_input_assembly_state_create_info.pNext                  = VK_NULL_HANDLE;
    vk_pipeline_input_assembly_state_create_info.flags                  = 0U;
    vk_pipeline_input_assembly_state_create_info.topology               = static_cast<VkPrimitiveTopology>(specification_.primitive_topology);
    vk_pipeline_input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;
    //------------------------------------------------------------------------
    // Rasterization State
    //------------------------------------------------------------------------

    /*  Configuration for the fixed-function rasterization.
        In here is where we enable or disable backface culling, and set line width or wireframe drawing. */
    VkPipelineRasterizationStateCreateInfo vk_pipeline_rasterization_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};

    vk_pipeline_rasterization_state_create_info.pNext                   = VK_NULL_HANDLE;
    vk_pipeline_rasterization_state_create_info.flags                   = 0U;
    vk_pipeline_rasterization_state_create_info.depthClampEnable        = specification_.is_depth_clamping;
    vk_pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    vk_pipeline_rasterization_state_create_info.polygonMode             = static_cast<VkPolygonMode>(specification_.primitive_polygon_mode);
    vk_pipeline_rasterization_state_create_info.cullMode =
        (specification_.is_back_face_culling) ? static_cast<VkCullModeFlags>(VK_CULL_MODE_BACK_BIT) : static_cast<VkCullModeFlags>(VK_CULL_MODE_NONE);
    vk_pipeline_rasterization_state_create_info.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vk_pipeline_rasterization_state_create_info.depthBiasEnable         = VK_TRUE;
    vk_pipeline_rasterization_state_create_info.depthBiasConstantFactor = specification_.depth_bias_constant_factor;
    vk_pipeline_rasterization_state_create_info.depthBiasClamp          = specification_.depth_bias_clamp;
    vk_pipeline_rasterization_state_create_info.depthBiasSlopeFactor    = specification_.depth_bias_slope_factor;
    vk_pipeline_rasterization_state_create_info.lineWidth               = specification_.line_width;

    //------------------------------------------------------------------------
    // Dynamic State Configuration
    //------------------------------------------------------------------------

    std::vector<VkDynamicState> vk_dynamic_states{
        VK_DYNAMIC_STATE_VIEWPORT,
        // VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
        VK_DYNAMIC_STATE_SCISSOR,
        // VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT
    };

    VkPipelineViewportStateCreateInfo vk_pipeline_viewport_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};

    vk_pipeline_viewport_state_create_info.pNext         = VK_NULL_HANDLE;
    vk_pipeline_viewport_state_create_info.flags         = 0U;
    vk_pipeline_viewport_state_create_info.viewportCount = 0U;
    vk_pipeline_viewport_state_create_info.pViewports    = VK_NULL_HANDLE;
    vk_pipeline_viewport_state_create_info.scissorCount  = 0U;
    vk_pipeline_viewport_state_create_info.pScissors     = VK_NULL_HANDLE;

    for (VkDynamicState const vk_dynamic_state : vk_dynamic_states)
    {
        if (vk_dynamic_state == VK_DYNAMIC_STATE_VIEWPORT)
        {
            vk_pipeline_viewport_state_create_info.viewportCount++;
        }

        if (vk_dynamic_state == VK_DYNAMIC_STATE_SCISSOR)
        {
            vk_pipeline_viewport_state_create_info.scissorCount++;
        }

        // TODO:
        /*if (instance->m_Specification.Topology == PrimitiveTopology::Lines || instance->m_Specification.Topology == PrimitiveTopology::LineStrip ||
           instance->m_Specification.Wireframe) dynamicStateEnables.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);*/
    }

    //------------------------------------------------------------------------
    // Dynamic State Create Info
    //------------------------------------------------------------------------
    VkPipelineDynamicStateCreateInfo vk_pipeline_dynamic_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};

    vk_pipeline_dynamic_state_create_info.pNext             = VK_NULL_HANDLE;
    vk_pipeline_dynamic_state_create_info.flags             = 0U;
    vk_pipeline_dynamic_state_create_info.dynamicStateCount = static_cast<std::uint32_t>(vk_dynamic_states.size());
    vk_pipeline_dynamic_state_create_info.pDynamicStates    = vk_dynamic_states.data();

    //------------------------------------------------------------------------
    // Depth Stencil State
    //------------------------------------------------------------------------
    // Actually we need check evereting from shader that we need to create bcs we can no create stensil or sometning.!
    VkPipelineDepthStencilStateCreateInfo vk_pipeline_depth_stencil_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

    vk_pipeline_depth_stencil_state_create_info.pNext                 = VK_NULL_HANDLE;
    vk_pipeline_depth_stencil_state_create_info.flags                 = 0U;
    vk_pipeline_depth_stencil_state_create_info.depthTestEnable       = specification_.is_depth_test_enabled;
    vk_pipeline_depth_stencil_state_create_info.depthWriteEnable      = specification_.is_depth_test_enabled;
    vk_pipeline_depth_stencil_state_create_info.depthCompareOp        = static_cast<VkCompareOp>(specification_.depth_test);
    vk_pipeline_depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
    vk_pipeline_depth_stencil_state_create_info.stencilTestEnable     = VK_FALSE;
    vk_pipeline_depth_stencil_state_create_info.front.failOp          = VK_STENCIL_OP_KEEP;
    vk_pipeline_depth_stencil_state_create_info.front.passOp          = VK_STENCIL_OP_KEEP;
    vk_pipeline_depth_stencil_state_create_info.front.compareOp       = VK_COMPARE_OP_ALWAYS;
    vk_pipeline_depth_stencil_state_create_info.back.failOp           = VK_STENCIL_OP_KEEP;
    vk_pipeline_depth_stencil_state_create_info.back.passOp           = VK_STENCIL_OP_KEEP;
    vk_pipeline_depth_stencil_state_create_info.back.compareOp        = VK_COMPARE_OP_NEVER;
    vk_pipeline_depth_stencil_state_create_info.minDepthBounds        = 0.f;
    vk_pipeline_depth_stencil_state_create_info.maxDepthBounds        = 1.f;

    //------------------------------------------------------------------------
    // Multisample State
    //------------------------------------------------------------------------
    /*  This allows us to configure MSAA for this pipeline. We are not going to use MSAA at this time,
        so we are going to default it to 1 sample and MSAA disabled.
        If you wanted to enable MSAA, you would need to set rasterizationSamples to more than 1,
        and enable sampleShading. Keep in mind that for MSAA to work, your renderpass also has to support it,
        which complicates things significantly. */

    VkPipelineMultisampleStateCreateInfo vk_pipeline_multisample_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};

    vk_pipeline_multisample_state_create_info.pNext                = VK_NULL_HANDLE;
    vk_pipeline_multisample_state_create_info.flags                = 0U;
    vk_pipeline_multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    vk_pipeline_multisample_state_create_info.sampleShadingEnable  = VK_FALSE;
    vk_pipeline_multisample_state_create_info.minSampleShading     = 0.2f; //.2f

    /*	A vertex binding describes at which rate to load data from memory throughout the vertices.
            It specifies the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance.
     */
    vk_vertex_input_binding_descriptions_.resize(specification_.vertex_layout.getElementLayout().size());

    for (std::size_t i{0U}; i < vk_vertex_input_binding_descriptions_.size(); ++i)
    {
        vk_vertex_input_binding_descriptions_[i].binding   = i;
        vk_vertex_input_binding_descriptions_[i].stride    = specification_.vertex_layout.getStride(i);
        vk_vertex_input_binding_descriptions_[i].inputRate = static_cast<VkVertexInputRate>(specification_.vertex_layout.getElementLayout()[i].usage);
    }

    /* The structure that describes how to handle vertex input. */
    std::vector<VkVertexInputAttributeDescription> vk_vertex_input_attribute_descriptions{specification_.vertex_layout.getCount()};

    std::uint32_t element_index{0U};
    for (std::uint32_t i{0U}; i < specification_.vertex_layout.getElementLayout().size(); ++i)
    {
        for (std::uint32_t j{0U}; j < specification_.vertex_layout.getElementLayout()[i].elements.size(); ++j)
        {
            vk_vertex_input_attribute_descriptions[element_index].location = element_index;
            vk_vertex_input_attribute_descriptions[element_index].binding  = i;
            vk_vertex_input_attribute_descriptions[element_index].format =
                VulkanShader::getShaderDataToVulkanFormat(specification_.vertex_layout.getElementLayout()[i].elements[j].shade_data_type);

            vk_vertex_input_attribute_descriptions[element_index].offset = specification_.vertex_layout.getElementLayout()[i].elements[j].offset;

            ++element_index;
        }
    }

    /* Contains the information for vertex buffers and vertex formats.
           This is equivalent to the VAO configuration on opengl.*/
    VkPipelineVertexInputStateCreateInfo vk_pipeline_vertex_input_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    vk_pipeline_vertex_input_state_create_info.pNext = VK_NULL_HANDLE;
    vk_pipeline_vertex_input_state_create_info.flags = 0U;
    vk_pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount =
        static_cast<std::uint32_t>(vk_vertex_input_binding_descriptions_.size());
    vk_pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = vk_vertex_input_binding_descriptions_.data();
    vk_pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount =
        static_cast<std::uint32_t>(vk_vertex_input_attribute_descriptions.size());
    vk_pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = vk_vertex_input_attribute_descriptions.data();

    std::vector<VkFormat> vk_color_formats;
    VkFormat              vk_depth_format{VK_FORMAT_UNDEFINED};

    for (Image::Specification const& image_specification : specification_.frame_buffer->getSpecification().attachments.texture_attachments)
    {
        if (vk_utils::isDepthFormat(image_specification.format) || vk_utils::isDepthStencilFormat(image_specification.format))
        {
            vk_depth_format = vk_utils::toVulkanImageFormat(image_specification.format);
        }
        else
        {
            vk_color_formats.emplace_back(vk_utils::toVulkanImageFormat(image_specification.format));
        }
    }

    vk_pipeline_rendering_create_info_.pNext                   = VK_NULL_HANDLE;
    vk_pipeline_rendering_create_info_.viewMask                = 0U;
    vk_pipeline_rendering_create_info_.colorAttachmentCount    = static_cast<uint32_t>(vk_color_formats.size());
    vk_pipeline_rendering_create_info_.pColorAttachmentFormats = vk_color_formats.data();
    vk_pipeline_rendering_create_info_.depthAttachmentFormat   = vk_depth_format;
    vk_pipeline_rendering_create_info_.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    //------------------------------------------------------------------------
    // Color Blend State
    //------------------------------------------------------------------------
    std::vector<VkPipelineColorBlendAttachmentState> vk_pipeline_color_blend_attachment_states{vk_color_formats.size()};

    for (VkPipelineColorBlendAttachmentState& vk_pipeline_color_blend_attachment_state : vk_pipeline_color_blend_attachment_states)
    {
        vk_pipeline_color_blend_attachment_state.blendEnable         = VK_FALSE;
        vk_pipeline_color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        vk_pipeline_color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        vk_pipeline_color_blend_attachment_state.colorBlendOp        = VK_BLEND_OP_ADD;
        vk_pipeline_color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        vk_pipeline_color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        vk_pipeline_color_blend_attachment_state.alphaBlendOp        = VK_BLEND_OP_ADD;
        vk_pipeline_color_blend_attachment_state.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    }

    VkPipelineColorBlendStateCreateInfo vk_pipeline_color_blend_state_create_info{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    vk_pipeline_color_blend_state_create_info.pNext              = VK_NULL_HANDLE;
    vk_pipeline_color_blend_state_create_info.flags              = 0U;
    vk_pipeline_color_blend_state_create_info.logicOpEnable      = VK_FALSE;
    vk_pipeline_color_blend_state_create_info.logicOp            = VK_LOGIC_OP_COPY;
    vk_pipeline_color_blend_state_create_info.attachmentCount    = static_cast<std::uint32_t>(vk_pipeline_color_blend_attachment_states.size());
    vk_pipeline_color_blend_state_create_info.pAttachments       = vk_pipeline_color_blend_attachment_states.data();
    vk_pipeline_color_blend_state_create_info.blendConstants[0U] = 0.f;
    vk_pipeline_color_blend_state_create_info.blendConstants[1U] = 0.f;
    vk_pipeline_color_blend_state_create_info.blendConstants[2U] = 0.f;
    vk_pipeline_color_blend_state_create_info.blendConstants[3U] = 0.f;
    //------------------------------------------------------------------------
    // Pipeline Create Info
    //------------------------------------------------------------------------
    VkGraphicsPipelineCreateInfo vk_graphics_pipeline_create_info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    // Dynamic rendering
    vk_graphics_pipeline_create_info.pNext = &vk_pipeline_rendering_create_info_;
    vk_graphics_pipeline_create_info.flags = 0U;
    vk_graphics_pipeline_create_info.stageCount =
        static_cast<std::uint32_t>(specification_.shader->as<VulkanShader>().getVkPipelineShaderStageCreateInfo().size());
    vk_graphics_pipeline_create_info.pStages             = specification_.shader->as<VulkanShader>().getVkPipelineShaderStageCreateInfo().data();
    vk_graphics_pipeline_create_info.pVertexInputState   = &vk_pipeline_vertex_input_state_create_info;
    vk_graphics_pipeline_create_info.pInputAssemblyState = &vk_pipeline_input_assembly_state_create_info;
    vk_graphics_pipeline_create_info.pTessellationState  = VK_NULL_HANDLE;
    vk_graphics_pipeline_create_info.pViewportState      = &vk_pipeline_viewport_state_create_info;
    vk_graphics_pipeline_create_info.pRasterizationState = &vk_pipeline_rasterization_state_create_info;
    vk_graphics_pipeline_create_info.pMultisampleState   = &vk_pipeline_multisample_state_create_info;
    vk_graphics_pipeline_create_info.pDepthStencilState  = &vk_pipeline_depth_stencil_state_create_info;
    vk_graphics_pipeline_create_info.pColorBlendState    = &vk_pipeline_color_blend_state_create_info;
    vk_graphics_pipeline_create_info.pDynamicState       = &vk_pipeline_dynamic_state_create_info;
    vk_graphics_pipeline_create_info.layout              = vk_pipeline_layout_;
    // Since we are using dynamic render we dont need render pass otherwise get it form frame buffer
    vk_graphics_pipeline_create_info.renderPass = VK_NULL_HANDLE;
    // Ignored
    vk_graphics_pipeline_create_info.subpass            = 0U;
    vk_graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    vk_graphics_pipeline_create_info.basePipelineIndex  = 0U;

    VK_CHECK_RESULT(vkCreateGraphicsPipelines(vk_logical_device, VK_NULL_HANDLE, 1U, &vk_graphics_pipeline_create_info,
                                              vulkan_instance.allocation_callbaks, &vk_pipeline_),
                    "Failed to create graphics pipeline!");
    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_PIPELINE>(vulkan_instance.instance, std::string("Vulkan render pipeline: " + specification_.name),
                                                          vk_logical_device, vk_pipeline_);

    VSHADE_CORE_DEBUG("Vulkan render pipeline: '{}' has been compiled successful.", specification_.name);
}

void vshade::render::VulkanRenderPipeline::bindDescriptorsSets(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
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
            // WARNING, potenital access from different frames into the same resources_ that can be modifyed be other frame !

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
    vkCmdBindDescriptorSets(vk_render_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout_,
                            static_cast<std::uint32_t>(Pipeline::Set::_GLOBAL_), static_cast<std::uint32_t>(Pipeline::Set::_MAX_ENUM_),
                            vk_descriptor_sets.data(), 0U, VK_NULL_HANDLE);
}

void vshade::render::VulkanRenderPipeline::updateResources(std::shared_ptr<RenderCommandBuffer> const render_command_buffer)
{
    VulkanRender::instance().enqueCommand([=](std::uint32_t const frame_index) { bindDescriptorsSets(render_command_buffer, frame_index); });
}

void vshade::render::VulkanRenderPipeline::bind(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::uint32_t const frame_index)
{
    uniform_offset_ = 0U;
    vkCmdBindPipeline(render_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer,
                      VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_);
}

void vshade::render::VulkanRenderPipeline::setStorageBuffer(std::shared_ptr<StorageBuffer const> const storage_buffer, Pipeline::Set const set,
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

void vshade::render::VulkanRenderPipeline::setUniformBuffer(std::shared_ptr<UniformBuffer const> const uniform_buffer, Pipeline::Set const set,
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

void vshade::render::VulkanRenderPipeline::setTexture(std::shared_ptr<Texture2D const> const texture, Pipeline::Set const set,
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

void vshade::render::VulkanRenderPipeline::setUniform(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::size_t const size,
                                                      void const* data, Shader::Stage const shader_stage)
{
    VulkanRender::instance().enqueCommand([=](std::uint32_t const frame_index)
                                          { setUniformRT(render_command_buffer, size, data, shader_stage, frame_index); });
}

void vshade::render::VulkanRenderPipeline::setUniformRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::size_t const size,
                                                        void const* data, Shader::Stage const shader_stage, std::uint32_t const frame_index)
{
    VkCommandBuffer vulkan_render_command_buffer{
        render_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer};

    vkCmdPushConstants(vulkan_render_command_buffer, vk_pipeline_layout_, VulkanShader::fromShaderStageToVkShaderType(shader_stage), uniform_offset_,
                       size, data);
    uniform_offset_ += size;
}

void vshade::render::VulkanRenderPipeline::recompile()
{
    if (auto shader = Shader::create(specification_.shader->getSpecification(), true))
    {
        specification_.shader = shader;
        invalidate();
    }
}
