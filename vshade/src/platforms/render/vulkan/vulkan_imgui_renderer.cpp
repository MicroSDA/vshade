#include "engine/platforms/render/vulkan/vulkan_imgui_renderer.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <engine/core/render/render.h>
#include <engine/core/window/window.h>
#include <engine/platforms/render/vulkan/vulkan_render_context.h>
#include <engine/platforms/render/vulkan/vulkan_swap_chain.h>
#include <glfw/include/GLFW/glfw3.h>

vshade::render::VulkanImGuiRenderer::VulkanImGuiRenderer(std::shared_ptr<vshade::render::FrameBuffer> const frame_buffer)
    : ImGuiRenderer{frame_buffer}
{
    VulkanInstance&        vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const         vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();
    VkPhysicalDevice const vk_physical_device =
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getVkDevice();

    //------------------------------------------------------------------------
    // ImGui frame buffer
    //------------------------------------------------------------------------
    // Create main frame buffer with one collor attachment (Assuming that
    // _BGRA8UN_ should be supported on any devices)
    //------------------------------------------------------------------------
    std::uint32_t const frames_in_flight{vshade::render::Render::instance().getFramesInFlightCount()};
    main_frame_buffer_ = !main_frame_buffer_ ? vshade::render::FrameBuffer::create(vshade::render::FrameBuffer::Specification{
                                                   2U, // Width
                                                   2U, // Height
                                                   {
                                                       {vshade::render::Image::Format::_BGRA8UN_, vshade::render::Image::Usage::_ATTACHMENT_,
                                                        vshade::render::Image::Clamp::_CLAMP_TO_EDGE_},
                                                       {vshade::render::Image::Format::_DEPTH_, vshade::render::Image::Usage::_ATTACHMENT_,
                                                        vshade::render::Image::Clamp::_CLAMP_TO_EDGE_},
                                                   },
                                                   glm::vec4{0.f, 0.f, 0.f, 1.f}})
                                             : main_frame_buffer_;

    //------------------------------------------------------------------------
    // Declare frame buffer as shared resource for all
    //------------------------------------------------------------------------
    vshade::LayerResourceManager::instance().declare("ImGuiRenderer frame buffer", &main_frame_buffer_);

    //------------------------------------------------------------------------
    // ImGui render command buffer
    //------------------------------------------------------------------------
    render_command_buffer_ =
        RenderCommandBuffer::create(render::RenderCommandBuffer::Type::_PRIMARY_, render::RenderCommandBuffer::Family::_GRAPHIC_, frames_in_flight);

    //------------------------------------------------------------------------
    // ImGui descriptors
    //------------------------------------------------------------------------
    // VkDescriptorPoolSize vk_descriptor_pool_size[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000U},
    //                                                   {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000U},
    //                                                   {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000U},
    //                                                   {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000U},
    //                                                   {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000U},
    //                                                   {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000U},
    //                                                   {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000U},
    //                                                   {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000U},
    //                                                   {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000U},
    //                                                   {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000U},
    //                                                   {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000U}};

    // VkDescriptorPoolCreateInfo vk_descriptor_pool_create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    // vk_descriptor_pool_create_info.pNext         = VK_NULL_HANDLE;
    // vk_descriptor_pool_create_info.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    // vk_descriptor_pool_create_info.maxSets       = 1000U;
    // vk_descriptor_pool_create_info.poolSizeCount = static_cast<std::uint32_t>(std::size(vk_descriptor_pool_size));
    // vk_descriptor_pool_create_info.pPoolSizes    = vk_descriptor_pool_size;

    // VK_CHECK_RESULT(vkCreateDescriptorPool(vk_logical_device, &vk_descriptor_pool_create_info, nullptr, &vk_descriptor_pool_),
    //                 "Failed to create descriptor pool");

    VkFormat vk_color_format{main_frame_buffer_->as<VulkanFrameBuffer>().getColorAttachment(0)->getImage()->as<VulkanImage2D>().getVkImageFormat()};
    VkFormat vk_depth_format{RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getDepthForamt()};

    static ImGui_ImplVulkan_InitInfo imgui_impl_vulkan_init_info;
    imgui_impl_vulkan_init_info.Instance       = vulkan_instance.instance;
    imgui_impl_vulkan_init_info.PhysicalDevice = vk_physical_device;
    imgui_impl_vulkan_init_info.Device         = vk_logical_device;
    imgui_impl_vulkan_init_info.Queue          = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVulkanQueus().graphic;
    // imgui_impl_vulkan_init_info.DescriptorPool     = vk_descriptor_pool_;
    imgui_impl_vulkan_init_info.DescriptorPoolSize = 1000U; // By specifying this we use own imgui descriptors
    imgui_impl_vulkan_init_info.MinImageCount      = 2U;
    imgui_impl_vulkan_init_info.ImageCount         = 2U;
    imgui_impl_vulkan_init_info.MSAASamples        = VK_SAMPLE_COUNT_1_BIT;
    imgui_impl_vulkan_init_info.Allocator          = vulkan_instance.allocation_callbaks;

    imgui_impl_vulkan_init_info.UseDynamicRendering = true; // Use dynamic rendering

    imgui_impl_vulkan_init_info.PipelineRenderingCreateInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    imgui_impl_vulkan_init_info.PipelineRenderingCreateInfo.pNext                = VK_NULL_HANDLE;
    imgui_impl_vulkan_init_info.PipelineRenderingCreateInfo.viewMask             = 0U;
    imgui_impl_vulkan_init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1U;

    imgui_impl_vulkan_init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &vk_color_format;
    imgui_impl_vulkan_init_info.PipelineRenderingCreateInfo.depthAttachmentFormat   = vk_depth_format;
    imgui_impl_vulkan_init_info.PipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    imgui_context_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(imgui_context_);
    ImGui::GetIO().BackendFlags &= ~ImGuiBackendFlags_RendererHasTextures;
    //------------------------------------------------------------------------
    // ImGui Vulkan init
    //------------------------------------------------------------------------
    ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(window::Window::instance().getWindowNativeHanlder()), true);
    ImGui_ImplVulkan_Init(&imgui_impl_vulkan_init_info);

    //------------------------------------------------------------------------
    // Create multithreaded helper
    //------------------------------------------------------------------------
    ImGuiThreadRenderData::create<ImGuiThreadRenderData>(frames_in_flight);
}

vshade::render::VulkanImGuiRenderer::~VulkanImGuiRenderer()
{
    for (auto [texture, descriptor] : imgui_texture_descriptors_)
    {
        ImGui_ImplVulkan_RemoveTexture(descriptor);
    }
    ImGui_ImplVulkan_Shutdown();
}

void vshade::render::VulkanImGuiRenderer::onUpdate(std::shared_ptr<Scene> const scene, time::FrameTimer const& frame_timer,
                                                   std::uint32_t const frame_index)
{
    //------------------------------------------------------------------------
    // Resize ImGui frame buffer depends on swapchain
    //------------------------------------------------------------------------
    if (!is_external_frame_buffer_)
    {
        std::pair<std::uint32_t, std::uint32_t> const resolution{vshade::render::SwapChain::instance().getResolution()};

        if (main_frame_buffer_->getWidth() != resolution.first || main_frame_buffer_->getHeight() != resolution.second)
        {
            main_frame_buffer_->resize(resolution.first, resolution.second);
        }
    }
}

void vshade::render::VulkanImGuiRenderer::onRender(std::shared_ptr<Scene> const scene, time::FrameTimer const& frame_timer,
                                                   std::uint32_t const frame_index)
{
    //------------------------------------------------------------------------
    // Prepare the data for rendering
    //------------------------------------------------------------------------
    ImGui::Render();

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    //------------------------------------------------------------------------
    // Save ImGui render data to call it in the render thread
    //------------------------------------------------------------------------
    // ImGuiThreadRenderData::instance().createSnapshot(ImGui::GetDrawData(), &ImGui::GetPlatformIO().Textures,
    //                                                  vshade::render::Render::instance().getCurrentPrepareFrameIndex(), ImGui::GetTime());
    ImGuiThreadRenderData::instance().waitAndSync();
    //------------------------------------------------------------------------
    // Enque ImGui render commands
    //------------------------------------------------------------------------
    
    Render::instance().enqueCommand(
        [=](std::uint32_t const frame_index_) mutable
        {
            auto& vk_rendering_info = main_frame_buffer_->as<VulkanFrameBuffer>().getVkRenderingInfo();
            auto  vulkan_command_buffer =
                render_command_buffer_->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index_).vk_command_buffer;

            render_command_buffer_->begin(frame_index_);

            vshade::render::Render::instance().beginTimestampRT(render_command_buffer_, frame_index_, "VulkanImGuiRenderer");
            vkCmdBeginRendering(vulkan_command_buffer, &vk_rendering_info);

            // TIP: Clearing buffer was disabled because at this moment we use same frame buffer for all renderers and ImGuiLayer pushed as
            //         overlay

            // VkClearRect vk_clear_rect{
            //     vk_rendering_info.renderArea,
            //     0U,
            //     vk_rendering_info.layerCount,
            // };

            // vkCmdClearAttachments(vulkan_command_buffer,
            //                       static_cast<std::uint32_t>(main_frame_buffer_->as<VulkanFrameBuffer>().getVkClearAttachments().size()),
            //                       main_frame_buffer_->as<VulkanFrameBuffer>().getVkClearAttachments().data(), 1U, &vk_clear_rect);

            // render_command_buffer_->getQueueMutex(RenderCommandBuffer::Family::_GRAPHIC_).lock();
            // ImGuiThreadRenderData::instance().processRender(frame_index_, vulkan_command_buffer);
            // render_command_buffer_->getQueueMutex(RenderCommandBuffer::Family::_GRAPHIC_).unlock();


            ImGui_ImplVulkan_UpdateTexture(*ImGui::GetPlatformIO().Textures.Data);
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vulkan_command_buffer);

            vkCmdEndRendering(vulkan_command_buffer);
            vshade::render::Render::instance().endTimestampRT(render_command_buffer_, frame_index_, "VulkanImGuiRenderer");

            render_command_buffer_->end(frame_index_);
            render_command_buffer_->submit(frame_index_);
        });
}

void vshade::render::VulkanImGuiRenderer::onRenderBegin(time::FrameTimer const& frame_timer, std::uint32_t const frame_index)
{
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplVulkan_NewFrame();

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize);
    ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    (void)io;

    ImGui::NewFrame();
}

void vshade::render::VulkanImGuiRenderer::onRenderEnd(time::FrameTimer const& frame_timer, std::uint32_t const frame_index)
{
    // Set as swapchain resolve image
    Render::instance().resolveIntoSwapChain(main_frame_buffer_->getColorAttachment());
}

void vshade::render::VulkanImGuiRenderer::drawTexture(std::shared_ptr<render::Texture2D> texture, ImVec2 const& size, ImVec4 const& borderColor,
                                                      float const alpha)
{
    // TODO: in case we want to add drawTexture per mip and per layer, we neet to chage imgui_texture_descriptors_ to handle that VkView also should
    // be enveolved into the hash
    VulkanImage2D const& vulkan_image = texture->getImage()->as<VulkanImage2D>();

    if (imgui_texture_descriptors_.find(texture) == imgui_texture_descriptors_.end())
    {
        imgui_texture_descriptors_.emplace(texture, VkDescriptorSet{
                                                        ImGui_ImplVulkan_AddTexture(texture->as<VulkanTexture2D>().getVkSampler(),
                                                                                    vulkan_image.getVkView(), vulkan_image.getVkLayout()),
                                                    });
    }
    ImGui::ImageWithBg(imgui_texture_descriptors_.at(texture), size, ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.0, 0.0, 0.0, alpha),
                       ImVec4(1.0, 1.0, 1.0, alpha));
}

bool vshade::render::VulkanImGuiRenderer::drawImageButton(char const* title, std::shared_ptr<render::Texture2D> texture, ImVec2 const& size,
                                                          ImVec4 const& border_color, float const alpha)
{
    VulkanImage2D const& vulkan_image = texture->getImage()->as<VulkanImage2D>();

    if (imgui_texture_descriptors_.find(texture) == imgui_texture_descriptors_.end())
    {
        imgui_texture_descriptors_.emplace(texture, VkDescriptorSet{
                                                        ImGui_ImplVulkan_AddTexture(texture->as<VulkanTexture2D>().getVkSampler(),
                                                                                    vulkan_image.getVkView(), vulkan_image.getVkLayout()),
                                                    });
    }
    return ImGui::ImageButton(title, imgui_texture_descriptors_.at(texture), size, ImVec2(1, 0), ImVec2(0, 1), ImVec4(0.0, 0.0, 0.0, 0.0),
                              ImVec4(1.0, 1.0, 1.0, alpha));
}