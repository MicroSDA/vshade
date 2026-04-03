#include "engine/platforms/render/vulkan/vulkan_swap_chain.h"
#include <glfw/include/GLFW/glfw3.h>

vshade::render::VulkanSwapChain::~VulkanSwapChain()
{
    //------------------------------------------------------------------------
    // Device Synchronization Before Destruction
    //------------------------------------------------------------------------

    VulkanInstance& vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const  vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();

    // Wait until the device has finished all queued operations before proceeding with cleanup
    vkDeviceWaitIdle(vk_logical_device);

    //------------------------------------------------------------------------
    // Swapchain Destruction
    //------------------------------------------------------------------------

    if (vk_swap_chain_ != nullptr)
    {
        vkDestroySwapchainKHR(vk_logical_device, vk_swap_chain_, vulkan_instance.allocation_callbaks);
    }

    //------------------------------------------------------------------------
    // Framebuffer Cleanup
    //------------------------------------------------------------------------

    frame_buffers_.clear();

    //------------------------------------------------------------------------
    // Semaphore Destruction
    //------------------------------------------------------------------------

    for (VulkanSemaphores vulkan_semaphores : vulkan_semaphores_)
    {
        if (vulkan_semaphores.render_complete != nullptr)
        {
            vkDestroySemaphore(vk_logical_device, vulkan_semaphores.render_complete, vulkan_instance.allocation_callbaks);
        }

        if (vulkan_semaphores.present_complete != nullptr)
        {
            vkDestroySemaphore(vk_logical_device, vulkan_semaphores.present_complete, vulkan_instance.allocation_callbaks);
        }
    }

    //------------------------------------------------------------------------
    // Surface Destruction
    //------------------------------------------------------------------------

    if (vk_surface_ != nullptr)
    {
        vkDestroySurfaceKHR(vulkan_instance.instance, vk_surface_, vulkan_instance.allocation_callbaks);
    }

    //------------------------------------------------------------------------
    // Command Buffer Reset (Commented Out)
    //------------------------------------------------------------------------

    swap_chain_command_buffer_.reset();

    //------------------------------------------------------------------------
    // Final Device Synchronization
    //------------------------------------------------------------------------

    vkDeviceWaitIdle(vk_logical_device);
}

bool vshade::render::VulkanSwapChain::onResizeEvent(std::shared_ptr<event::WindowResize> const event)
{
    is_resize_requested_.store(true, std::memory_order_release);
    swaph_chain_resolution_.store(packResoulution(event->getWidth(), event->getHeight()), std::memory_order_release);

    return false;
}

void vshade::render::VulkanSwapChain::resize(std::uint32_t const width, std::uint32_t const height)
{
    VkDevice const vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();

    vkDeviceWaitIdle(vk_logical_device);

    // createSurface(glfwGetCurrentContext());
    createFrame(width, height, frames_in_flyght_, v_sync_);

    vkDeviceWaitIdle(vk_logical_device);
}

std::pair<std::uint32_t, std::uint32_t> vshade::render::VulkanSwapChain::createFrame(std::uint32_t const width, std::uint32_t const height,
                                                                                     std::uint32_t const frames_in_flyght, bool const v_sync)
{
    std::pair<std::uint32_t, std::uint32_t> resoulution{0U, 0U};
    //------------------------------------------------------------------------
    // Initialization and Configuration
    //------------------------------------------------------------------------

    // Set the VSync and FramesCount settings
    v_sync_ = v_sync, frames_in_flyght_ = frames_in_flyght;

    VulkanInstance&        vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const         vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();
    VkPhysicalDevice const vk_physical_device =
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getVkDevice();
    VkSwapchainKHR vk_old_swapchain = vk_swap_chain_;

    //------------------------------------------------------------------------
    // Surface Capabilities and Present Modes
    //------------------------------------------------------------------------

    // Get physical device surface properties and formats
    VkSurfaceCapabilitiesKHR vk_surface_capabilities;
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface_, &vk_surface_capabilities),
                    "Failed to get physical device surface capabilities!");

    // Check if the surface size is zero, indicating that the window might be minimized
    if (!vk_surface_capabilities.currentExtent.width && !vk_surface_capabilities.currentExtent.height)
    {
        // Skip the swapchain creation if the window is minimized
        VSHADE_CORE_DEBUG("Skipping VulkanSwapChain::createFrame, VkSurfaceCapabilitiesKHR::curentExtent :{0}, {0}",
                          vk_surface_capabilities.currentExtent.width, vk_surface_capabilities.currentExtent.height);
        return resoulution;
    }

    // Get available present modes for the physical device
    std::uint32_t present_mode_count{0};
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface_, &present_mode_count, NULL),
                    "Failed to get physical device surface present modes!");
    if (!present_mode_count)
    {
        VSHADE_CORE_ERROR("Physical device surface present modes is: {}", present_mode_count);
    }

    // Retrieve the available present modes
    std::vector<VkPresentModeKHR> present_modes{present_mode_count};
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface_, &present_mode_count, present_modes.data()),
                    "Failed to get physical device surface present modes!");

    //------------------------------------------------------------------------
    // Swapchain Extent Configuration
    //------------------------------------------------------------------------

    // Determine the swapchain extent (resolution)

    // If the surface size is undefined, set the swapchain extent to the requested image size
    if (vk_surface_capabilities.currentExtent.width == ~0U)
    {
        vk_surface_capabilities.currentExtent.width  = width;
        vk_surface_capabilities.currentExtent.height = height;
        swaph_chain_resolution_.store(packResoulution(width, height), std::memory_order_release);
    }
    else
    {
        resoulution.first  = vk_surface_capabilities.currentExtent.width;
        resoulution.second = vk_surface_capabilities.currentExtent.height;
        swaph_chain_resolution_.store(packResoulution(width, height), std::memory_order_release);
    }

    VSHADE_CORE_DEBUG("SwapChain createFrame, width: {0}, height: {1}", width, height)

    //------------------------------------------------------------------------
    // Present Mode Selection
    //------------------------------------------------------------------------

    // Select a present mode for the swapchain
    VkPresentModeKHR vk_swap_chain_present_mode{VK_PRESENT_MODE_FIFO_KHR}; // Default mode with v-sync

    // If v-sync is not requested, look for a low-latency present mode
    if (!v_sync_)
    {
        for (VkPresentModeKHR const& present_mode : present_modes)
        {
            // Prefer MAILBOX mode for its low latency
            if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                vk_swap_chain_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
            // Fall back to IMMEDIATE mode if MAILBOX is not available
            if ((vk_swap_chain_present_mode != VK_PRESENT_MODE_MAILBOX_KHR) && (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR))
            {
                vk_swap_chain_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    //------------------------------------------------------------------------
    // Swapchain Image Configuration
    //------------------------------------------------------------------------

    // Determine the number of swapchain images
    std::uint32_t desired_number_of_swapchain_images{vk_surface_capabilities.minImageCount + 1U};
    if ((vk_surface_capabilities.maxImageCount > 0U) && (desired_number_of_swapchain_images > vk_surface_capabilities.maxImageCount))
    {
        // Ensure the number of images doesn't exceed the maximum supported
        desired_number_of_swapchain_images = vk_surface_capabilities.maxImageCount;
    }

    // Select the surface transformation, preferring no rotation if supported
    VkSurfaceTransformFlagsKHR vk_surface_pre_transform_flags;
    if (vk_surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        vk_surface_pre_transform_flags = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        // Fall back to the current transform if identity is not supported
        vk_surface_pre_transform_flags = vk_surface_capabilities.currentTransform;
    }

    // Choose a supported composite alpha format
    VkCompositeAlphaFlagBitsKHR                vk_composite_alpha{VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR}; // Default to opaque
    std::array<VkCompositeAlphaFlagBitsKHR, 4> vk_composite_alpha_flags{
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };

    // Select the first supported composite alpha format
    for (VkCompositeAlphaFlagBitsKHR const& vk_composite_alpha_flag : vk_composite_alpha_flags)
    {
        if (vk_surface_capabilities.supportedCompositeAlpha & vk_composite_alpha_flag)
        {
            vk_composite_alpha = vk_composite_alpha_flag;
            break;
        }
    }

    //------------------------------------------------------------------------
    // Swapchain Creation
    //------------------------------------------------------------------------

    // Create the swapchain with the selected settings
    VkSwapchainCreateInfoKHR vk_swapchain_create_info_khr{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    vk_swapchain_create_info_khr.pNext                 = VK_NULL_HANDLE;
    vk_swapchain_create_info_khr.flags                 = 0U;
    vk_swapchain_create_info_khr.surface               = vk_surface_;
    vk_swapchain_create_info_khr.minImageCount         = desired_number_of_swapchain_images;
    vk_swapchain_create_info_khr.imageFormat           = vk_color_format_;
    vk_swapchain_create_info_khr.imageColorSpace       = vk_color_space_;
    vk_swapchain_create_info_khr.imageExtent           = vk_surface_capabilities.currentExtent;
    vk_swapchain_create_info_khr.imageArrayLayers      = 1U;
    vk_swapchain_create_info_khr.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vk_swapchain_create_info_khr.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    vk_swapchain_create_info_khr.queueFamilyIndexCount = 0U;
    vk_swapchain_create_info_khr.pQueueFamilyIndices   = VK_NULL_HANDLE;
    vk_swapchain_create_info_khr.preTransform          = static_cast<VkSurfaceTransformFlagBitsKHR>(vk_surface_pre_transform_flags);
    vk_swapchain_create_info_khr.compositeAlpha        = vk_composite_alpha;
    vk_swapchain_create_info_khr.presentMode           = vk_swap_chain_present_mode;
    // clipped, allows the implementation to discard rendering outside of the surface area
    vk_swapchain_create_info_khr.clipped      = VK_TRUE;
    vk_swapchain_create_info_khr.oldSwapchain = vk_old_swapchain;

    // Enable additional image usage flags if supported
    if (vk_surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
    {
        vk_swapchain_create_info_khr.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (vk_surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
    {
        vk_swapchain_create_info_khr.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    // Create the swapchain
    VK_CHECK_RESULT(vkCreateSwapchainKHR(vk_logical_device, &vk_swapchain_create_info_khr, vulkan_instance.allocation_callbaks, &vk_swap_chain_),
                    "Failed to create a swapchain!");

    // Clean up the old swapchain if it exists
    if (vk_old_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(vk_logical_device, vk_old_swapchain, vulkan_instance.allocation_callbaks);
    }

    // Destroy old semaphores
    for (VulkanSemaphores vulkan_semaphores : vulkan_semaphores_)
    {
        if (vulkan_semaphores.render_complete != nullptr)
        {
            vkDestroySemaphore(vk_logical_device, vulkan_semaphores.render_complete, vulkan_instance.allocation_callbaks);
        }

        if (vulkan_semaphores.present_complete != nullptr)
        {
            vkDestroySemaphore(vk_logical_device, vulkan_semaphores.present_complete, vulkan_instance.allocation_callbaks);
        }
    }

    vulkan_semaphores_.clear();

    // Retrieve the number of swapchain images
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(vk_logical_device, vk_swap_chain_, &image_count_, VK_NULL_HANDLE),
                    "Failed to get swap chain images khr!");

    if (image_count_ < frames_in_flyght_)
    {
        // TODO: Warning and set image_count_ as global frames in fight
        VSHADE_CORE_ERROR("Swap chain support only '{0}' frames in flight!", image_count_)
    }

    // frames_in_flyght_ = image_count_;
    std::vector<VkImage> vk_swap_chain_images{image_count_};

    // Do we need to use image_count_ if we have frames_in_flyght_ ?
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(vk_logical_device, vk_swap_chain_, &image_count_, vk_swap_chain_images.data()),
                    "Failed to get swap chain images khr!");

    //------------------------------------------------------------------------
    // Framebuffer Setup
    //------------------------------------------------------------------------

    frame_buffers_.clear();

    for (std::uint32_t i{0U}; i < image_count_; ++i)
    {
        // Prepare attachments and frame buffer specifications
        std::vector<std::shared_ptr<Image2D>> images;
        Image::Specification                  color_attachment_specification;
        FrameBuffer::Specification            frame_buffer_specification;

        // Set color attachment specifications
        color_attachment_specification.width     = vk_surface_capabilities.currentExtent.width;
        color_attachment_specification.height    = vk_surface_capabilities.currentExtent.height;
        color_attachment_specification.layers    = 1U;
        color_attachment_specification.mip_count = 1U;
        color_attachment_specification.usage     = Image::Usage::_ATTACHMENT_;
        color_attachment_specification.format    = vk_utils::fromVulkanImageFormat(vk_color_format_);

        // Set color attachment specifications
        images.emplace_back(Image2D::create(color_attachment_specification, reinterpret_cast<void const*>(vk_swap_chain_images[i])));
        frame_buffer_specification.attachments.texture_attachments.emplace_back(color_attachment_specification);
        // Set depth-stencil attachment specifications
        color_attachment_specification.format = vk_utils::fromVulkanImageFormat(
            RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getDepthForamt());
        images.emplace_back(Image2D::create(color_attachment_specification));
        frame_buffer_specification.attachments.texture_attachments.emplace_back(color_attachment_specification);

        // Set frame buffer specifications
        frame_buffer_specification.width  = vk_surface_capabilities.currentExtent.width;
        frame_buffer_specification.height = vk_surface_capabilities.currentExtent.height;

        frame_buffer_specification.clear_color[0] = 0.0f;
        frame_buffer_specification.clear_color[1] = 0.0f;
        frame_buffer_specification.clear_color[2] = 0.0f;
        frame_buffer_specification.clear_color[3] = 1.0f;

        frame_buffers_.emplace_back(FrameBuffer::create(frame_buffer_specification, std::move(images)));
    }

    //------------------------------------------------------------------------
    // Command Buffer Creation
    //------------------------------------------------------------------------

    if (swap_chain_command_buffer_ == nullptr)
    {
        swap_chain_command_buffer_ =
            RenderCommandBuffer::create(RenderCommandBuffer::Type::_PRIMARY_, RenderCommandBuffer::Family::_GRAPHIC_, frames_in_flyght_);
    }

    //------------------------------------------------------------------------
    // Synchronization Objects Creation
    //------------------------------------------------------------------------

    // Create synchronization semaphores if they do not already exist
    if (!vulkan_semaphores_.size())
    {
        for (std::uint32_t i{0U}; i < frames_in_flyght_; ++i)
        {
            VulkanSemaphores& vulkan_semaphores = vulkan_semaphores_.emplace_back();

            // Create semaphores for rendering and presentation synchronization
            VkSemaphoreCreateInfo vk_semaphore_create_info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            vk_semaphore_create_info.pNext = VK_NULL_HANDLE;
            vk_semaphore_create_info.flags = 0U;

            // Create render semaphore
            VK_CHECK_RESULT(vkCreateSemaphore(vk_logical_device, &vk_semaphore_create_info, vulkan_instance.allocation_callbaks,
                                              &vulkan_semaphores.render_complete),
                            "Failed to create semaphore!");
            // Create present semaphore
            VK_CHECK_RESULT(vkCreateSemaphore(vk_logical_device, &vk_semaphore_create_info, vulkan_instance.allocation_callbaks,
                                              &vulkan_semaphores.present_complete),
                            "Failed to create semaphore!");

            vk_utils::setDebugObjectName<VK_OBJECT_TYPE_SEMAPHORE>(vulkan_instance.instance, "Swapchain semaphore render complete", vk_logical_device,
                                                                   vulkan_semaphores.render_complete);
            vk_utils::setDebugObjectName<VK_OBJECT_TYPE_SEMAPHORE>(vulkan_instance.instance, "Swapchain semaphore present complete",
                                                                   vk_logical_device, vulkan_semaphores.present_complete);
        }
    }
    return resoulution;
}

void vshade::render::VulkanSwapChain::createSurface(void* window_native_handle)
{
    //------------------------------------------------------------------------
    // Retrieve Physical Device
    //------------------------------------------------------------------------

    VulkanInstance&        vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const         vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();
    VkPhysicalDevice const vk_physical_device =
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getVkDevice();

    //------------------------------------------------------------------------
    // Create Window Surface
    //------------------------------------------------------------------------

    // Create a Vulkan surface for rendering to the specified window
    VK_CHECK_RESULT(glfwCreateWindowSurface(vulkan_instance.instance, static_cast<GLFWwindow*>(window_native_handle),
                                            vulkan_instance.allocation_callbaks, &vk_surface_),
                    "Failed to create a window surface!");

    //------------------------------------------------------------------------
    // Retrieve Queue Family Properties
    //------------------------------------------------------------------------

    // Retrieve the number of queue families available on the physical device
    std::uint32_t queue_count{0U};
    vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queue_count, VK_NULL_HANDLE);

    // Error handling if no queue families are found
    if (!queue_count)
    {
        VSHADE_CORE_ERROR("Queue family count :{}!", queue_count);
    }

    // Retrieve properties for each queue family
    std::vector<VkQueueFamilyProperties> vk_queue_family_properties{queue_count};
    vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queue_count, vk_queue_family_properties.data());

    //------------------------------------------------------------------------
    // Check Queue Support for Presenting
    //------------------------------------------------------------------------

    // Check which queue families support presentation to the surface
    std::vector<VkBool32> supports_present(queue_count);
    for (std::uint32_t i{0U}; i < queue_count; ++i)
    {
        VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, i, vk_surface_, &supports_present[i]),
                        "Failed to get physical device surface support!");
    }

    //------------------------------------------------------------------------
    // Find Suitable Queue Families
    //------------------------------------------------------------------------

    // Find a queue family that supports both graphics and presentation
    std::uint32_t graphics_queue_node_index{UINT32_MAX};
    std::uint32_t present_queue_node_index{UINT32_MAX};

    for (std::uint32_t i{0U}; i < queue_count; i++)
    {
        // Check if the queue supports graphics operations
        if ((vk_queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            // If this queue also supports presentation, use it for both
            if (graphics_queue_node_index == UINT32_MAX)
            {
                graphics_queue_node_index = i;
            }

            if (supports_present[i] == VK_TRUE)
            {
                graphics_queue_node_index = i;
                present_queue_node_index  = i;
                break;
            }
        }
    }

    // If no single queue supports both, look for a separate presentation queue
    if (present_queue_node_index == UINT32_MAX)
    {
        for (std::uint32_t i{0U}; i < queue_count; ++i)
        {
            if (supports_present[i] == VK_TRUE)
            {
                present_queue_node_index = i;
                break;
            }
        }
    }

    // Error handling if no suitable queues are found
    if (graphics_queue_node_index == UINT32_MAX || present_queue_node_index == UINT32_MAX)
    {
        VSHADE_CORE_ERROR("Graphics queue node index or present queue node index were not been found!");
    }

    //------------------------------------------------------------------------
    // Retrieve Queue Handles
    //------------------------------------------------------------------------

    // Store the queue family index that supports presentation
    RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->setQueueFamilyIndexPresent(present_queue_node_index);

    // Retrieve the Vulkan queue for presentation
    vkGetDeviceQueue(vk_logical_device, present_queue_node_index, 0U,
                     &RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVulkanQueus().present);

    // Store the index of the graphics queue
    graphics_queue_node_index_ = graphics_queue_node_index;

    //------------------------------------------------------------------------
    // Retrieve Image Format and Color Space
    //------------------------------------------------------------------------

    getImageFormatAndColorSpace();
}

void vshade::render::VulkanSwapChain::beginFrame(std::uint32_t frame_index)
{
    if (is_resize_requested_.load(std::memory_order_acquire))
    {
        std::pair<std::uint32_t, std::uint32_t> resolution{SwapChain::unpacResoulution(swaph_chain_resolution_.load(std::memory_order_acquire))};
        resize(resolution.first, resolution.second);
        is_resize_requested_.store(false, std::memory_order_release);
    }

    acquireNextImage(std::chrono::nanoseconds::max(), frame_index);

    VkCommandBuffer vk_command_buffer =
        swap_chain_command_buffer_->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer;

    swap_chain_command_buffer_->begin(frame_index);

    std::shared_ptr<Image2D> image{frame_buffers_.at(current_image_index_)->as<VulkanFrameBuffer>().getColorAttachment()->getImage()};

    // Perform a layout transition on the image to ensure it's in the correct layout before blit
    image->as<VulkanImage2D>().layoutTransition(vk_command_buffer,                             // Command buffer used for the layout transition
                                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,          // New image layout for render or blit
                                                VK_ACCESS_NONE_KHR,                            // Source access mask (from Present'а reads GPU)
                                                VK_ACCESS_TRANSFER_WRITE_BIT,                  // Destination access — (upcoming access)
                                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // End of Present
                                                VK_PIPELINE_STAGE_TRANSFER_BIT,                // Wherer: render
                                                VK_IMAGE_ASPECT_COLOR_BIT,                     // Aspect mask (color aspect)
                                                0U, 1U, 0U, 1U                                 // Mip level and layer ranges
    );

    // clearSwapChainImage(vk_command_buffer);
}

void vshade::render::VulkanSwapChain::endFrame(std::uint32_t frame_index)
{
    // Renderer::endFrame(current_frame_index_);
    // Renderer::queryResults(current_frame_index_);

    VkCommandBuffer vk_command_buffer =
        swap_chain_command_buffer_->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer;

    // ------------------------------------------------------------------------
    // Layout transition
    // ------------------------------------------------------------------------
    std::shared_ptr<Image2D> image{frame_buffers_.at(current_image_index_)->as<VulkanFrameBuffer>().getColorAttachment()->getImage()};
    // Perform a layout transition on the image to ensure it's in the correct layout before presenting
    image->as<VulkanImage2D>().layoutTransition(vk_command_buffer,                    // Command buffer used for the layout transition
                                                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,      // New image layout for presentation
                                                VK_ACCESS_TRANSFER_WRITE_BIT,         // Source access mask (previous access)
                                                VK_ACCESS_NONE_KHR,                   // Destination access mask (upcoming access)
                                                VK_PIPELINE_STAGE_TRANSFER_BIT,       // Source pipeline stage (fragment shader)
                                                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // Destination pipeline stage (transfer operation)
                                                VK_IMAGE_ASPECT_COLOR_BIT,            // Aspect mask (color aspect)
                                                0U, 1U, 0U, 1U                        // Mip level and layer ranges
    );

    swap_chain_command_buffer_->end(frame_index);
}

bool vshade::render::VulkanSwapChain::present(std::chrono::nanoseconds timeout, std::uint32_t frame_index)
{
    // ------------------------------------------------------------------------
    // Image Layout Transition
    // ------------------------------------------------------------------------

    // Retrieve the Vulkan logical device to be used for issuing commands
    VkDevice const vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();

    // ------------------------------------------------------------------------
    // Command Buffer Submission
    // ------------------------------------------------------------------------

    // Retrieve the primary command buffer for the current frame
    VkCommandBuffer vk_command_buffer =
        swap_chain_command_buffer_->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer;

    // Define the pipeline stage where the image layout transition waits (color attachment output)
    VkPipelineStageFlags const vk_wait_stage_mask{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // TODO : CHeck this !!!

    // Retrieve the fence associated with the current frame to synchronize GPU execution
    VkFence vk_fence = swap_chain_command_buffer_->as<VulkanRenderCommandBuffer>().getVkFence(frame_index);

    // Fill the structure with information about the command buffer submission to the graphics queue
    VkSubmitInfo vk_submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    vk_submit_info.pNext              = VK_NULL_HANDLE;
    vk_submit_info.waitSemaphoreCount = 1U; // Number of semaphores to wait on before executing the command
    vk_submit_info.pWaitSemaphores =
        &vulkan_semaphores_.at(frame_index).present_complete; // Semaphores to wait on before executing the command buffer
    vk_submit_info.pWaitDstStageMask    = &vk_wait_stage_mask;
    vk_submit_info.commandBufferCount   = 1U; // Number of command buffers to submit
    vk_submit_info.pCommandBuffers      = &vk_command_buffer;
    vk_submit_info.signalSemaphoreCount = 1U; // Number of semaphores to signal once command buffer execution finishes
    vk_submit_info.pSignalSemaphores =
        &vulkan_semaphores_.at(frame_index).render_complete; // Semaphores to signal once the command buffer execution finishes

    // Reset the fence to prepare it for the next GPU synchronization
    VK_CHECK_RESULT(vkResetFences(vk_logical_device, 1, &vk_fence), "Failed to reset fence!");

    swap_chain_command_buffer_->getQueueMutex(RenderCommandBuffer::Family::_PRESENT_).lock();
    // Submit the command buffer to the graphics queue for execution
    VK_CHECK_RESULT(vkQueueSubmit(RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVulkanQueus().graphic, 1U,
                                  &vk_submit_info, vk_fence),
                    "Present failed to submit!");
    swap_chain_command_buffer_->getQueueMutex(RenderCommandBuffer::Family::_PRESENT_).unlock();

    //------------------------------------------------------------------------
    // Presenting the Image to the Swapchain
    //------------------------------------------------------------------------

    // Prepare the present info structure to present the image to the swap chain
    VkResult         vk_result{VK_RESULT_MAX_ENUM};
    VkPresentInfoKHR vk_present_info_khr{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    vk_present_info_khr.pNext              = VK_NULL_HANDLE;
    vk_present_info_khr.waitSemaphoreCount = 1U;
    vk_present_info_khr.pWaitSemaphores    = &vulkan_semaphores_.at(frame_index).render_complete; // Semaphore to wait on before presenting the image
    vk_present_info_khr.swapchainCount     = 1U;                                                  // Number of swapchains to present to
    vk_present_info_khr.pSwapchains        = &vk_swap_chain_;
    vk_present_info_khr.pImageIndices      = &current_image_index_; // Pointer to the index of the image to present
    vk_present_info_khr.pResults           = &vk_result;

    // Present the image to the swapchain
    swap_chain_command_buffer_->getQueueMutex(RenderCommandBuffer::Family::_PRESENT_).lock();
    vkQueuePresentKHR(RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVulkanQueus().present, &vk_present_info_khr);
    swap_chain_command_buffer_->getQueueMutex(RenderCommandBuffer::Family::_PRESENT_).unlock();

    updtaePresentDeltaTime();

    // Handle potential presentation errors
    if (vk_result != VK_SUCCESS)
    {
        // If the swapchain is out of date or suboptimal, trigger a resize
        if (vk_result == VK_ERROR_OUT_OF_DATE_KHR || vk_result == VK_SUBOPTIMAL_KHR)
        {
            // mby call on resize later
            std::pair<std::uint32_t, std::uint32_t> resolution{SwapChain::unpacResoulution(swaph_chain_resolution_.load(std::memory_order_acquire))};
            resize(resolution.first, resolution.second);
        }
        else
        {
            VK_CHECK_RESULT(vk_result, "Failed to present image!");
        }
    }

    //------------------------------------------------------------------------
    // Frame Synchronization and Index Management
    //------------------------------------------------------------------------

    // Wait for the fence to ensure the GPU has finished processing the current frame
    vk_result = vkWaitForFences(vk_logical_device, 1U, &vk_fence, VK_TRUE, timeout.count());

    VK_CHECK_RESULT(vk_result, std::to_string(timeout.count()).c_str());

    swap_chain_command_buffer_->setAsUnrecordered(frame_index);

    return vk_result == VK_SUCCESS;
}

void vshade::render::VulkanSwapChain::getImageFormatAndColorSpace()
{
    //------------------------------------------------------------------------
    // Retrieve Physical Device
    //------------------------------------------------------------------------
    VkPhysicalDevice const vk_physical_device =
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getVkDevice();
    //------------------------------------------------------------------------
    // Get Supported Surface Formats
    //------------------------------------------------------------------------

    std::uint32_t format_count{0U};
    // Query the number of supported surface formats
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface_, &format_count, VK_NULL_HANDLE),
                    "Failed to get physical device surface formats khr!");

    // Retrieve the list of supported surface formats
    std::vector<VkSurfaceFormatKHR> vk_surface_formats(format_count);
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface_, &format_count, vk_surface_formats.data()),
                    "Failed to get physical device surface formats khr!");

    //------------------------------------------------------------------------
    // Determine Preferred Surface Format
    //------------------------------------------------------------------------

    // If only one format is available and it is VK_FORMAT_UNDEFINED
    // assume VK_FORMAT_B8G8R8A8_UNORM as the preferred format
    if (format_count == 1U && vk_surface_formats[0U].format == VK_FORMAT_UNDEFINED)
    {
        vk_color_format_ = VK_FORMAT_B8G8R8A8_UNORM;
        vk_color_space_  = vk_surface_formats[0U].colorSpace;
    }
    else
    {
        // Check if VK_FORMAT_B8G8R8A8_UNORM is available in the list
        bool b8g8r8a8_unorm{false};
        for (VkSurfaceFormatKHR const& vk_surface_format : vk_surface_formats)
        {
            if (vk_surface_format.format == VK_FORMAT_B8G8R8A8_UNORM)
            {
                vk_color_format_ = vk_surface_format.format;
                vk_color_space_  = vk_surface_format.colorSpace;
                b8g8r8a8_unorm   = true;
                break;
            }
        }
        // If VK_FORMAT_B8G8R8A8_UNORM is not available, use the first format in the list
        if (!b8g8r8a8_unorm)
        {
            vk_color_format_ = vk_surface_formats[0U].format;
            vk_color_space_  = vk_surface_formats[0U].colorSpace;
        }
    }
}

void vshade::render::VulkanSwapChain::acquireNextImage(std::chrono::nanoseconds timeout, std::uint32_t frame_index)
{
    //------------------------------------------------------------------------
    // Acquiring the Next Swapchain Image
    //------------------------------------------------------------------------

    // Check if the swapchain is valid before attempting to acquire the next image
    if (vk_swap_chain_ != VK_NULL_HANDLE)
    {
        // Acquire the next image from the swapchain
        // This function returns the index of the next available image in the swapchain
        // The semaphore 'present_complete' is signaled when the image is ready to be used
        // Timeout in nanoseconds (UINT64_MAX disables timeout)

        VkResult vk_result =
            vkAcquireNextImageKHR(RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice(), vk_swap_chain_,
                                  timeout.count(), vulkan_semaphores_.at(frame_index).present_complete, VK_NULL_HANDLE, &current_image_index_);

        if (vk_result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            std::pair<std::uint32_t, std::uint32_t> resolution{SwapChain::unpacResoulution(swaph_chain_resolution_.load(std::memory_order_acquire))};
            resize(resolution.first, resolution.second);
        }
    }
    else
    {
        VSHADE_CORE_ERROR("Vulkan swap chain is nullptr!")
    }
}

void vshade::render::VulkanSwapChain::updtaePresentDeltaTime()
{
    static auto last_present_time{std::chrono::high_resolution_clock::now()};
    auto        now{std::chrono::high_resolution_clock::now()};
    present_delta_time_ = std::chrono::duration_cast<std::chrono::nanoseconds>(now - last_present_time).count();

    last_present_time = now;
}

void vshade::render::VulkanSwapChain::clearSwapChainImage(VkCommandBuffer vk_command_buffer)
{
    VulkanFrameBuffer&     vulkan_frame_buffer{frame_buffers_.at(current_image_index_)->as<VulkanFrameBuffer>()};
    VkRenderingInfo const& vk_rendering_info{vulkan_frame_buffer.getVkRenderingInfo()};

    vkCmdBeginRendering(vk_command_buffer, &vk_rendering_info);

    VkClearRect vk_clear_rect{
        vk_rendering_info.renderArea,
        0U,
        vk_rendering_info.layerCount,
    };

    vkCmdClearAttachments(vk_command_buffer, vulkan_frame_buffer.getVkClearAttachments().size(), vulkan_frame_buffer.getVkClearAttachments().data(),
                          1U, &vk_clear_rect);

    vkCmdEndRendering(vk_command_buffer);
}

void vshade::render::VulkanSwapChain::resolveIntoSwapChain(std::shared_ptr<Texture2D const> const texture, std::uint32_t const frame_index,
                                                           Texture2D::Filtration const filtration)
{
    VkCommandBuffer vk_command_buffer =
        swap_chain_command_buffer_->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer;

    std::shared_ptr<Image2D> dst_image{frame_buffers_.at(current_image_index_)->as<VulkanFrameBuffer>().getColorAttachment()->getImage()};

    static VkImageBlit vk_blit_region{};
    vk_blit_region.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    vk_blit_region.srcSubresource.mipLevel       = 0U;
    vk_blit_region.srcSubresource.baseArrayLayer = 0U;
    vk_blit_region.srcSubresource.layerCount     = 1U;
    vk_blit_region.srcOffsets[0U]                = {0U, 0U, 0U};
    vk_blit_region.srcOffsets[1U]                = {static_cast<std::int32_t>(texture->getImage()->getSpecification().width),
                                                    static_cast<std::int32_t>(texture->getImage()->getSpecification().height), 1U};

    vk_blit_region.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    vk_blit_region.dstSubresource.mipLevel       = 0U;
    vk_blit_region.dstSubresource.baseArrayLayer = 0U;
    vk_blit_region.dstSubresource.layerCount     = 1U;
    vk_blit_region.dstOffsets[0U]                = {0U, 0U, 0U};
    vk_blit_region.dstOffsets[1U]                = {static_cast<std::int32_t>(dst_image->getSpecification().width),
                                                    static_cast<std::int32_t>(dst_image->getSpecification().height), 1U};

    vkCmdBlitImage(vk_command_buffer,
                   texture->getImage()->as<VulkanImage2D>().getVkImage(),  // Src image
                   texture->getImage()->as<VulkanImage2D>().getVkLayout(), // Src layout
                   dst_image->as<VulkanImage2D>().getVkImage(),            // Dst image
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,                   // Dst layout
                   1U,                                                     // Region count
                   &vk_blit_region,                                        // Region
                   static_cast<VkFilter>(filtration)                       // Filtration
    );
}
