#include "engine/platforms/render/vulkan/vulkan_render_command_buffer.h"
#include <engine/platforms/render/vulkan/vulkan_render_context.h>

vshade::render::VulkanRenderCommandBuffer::VulkanRenderCommandBuffer(RenderCommandBuffer::Type const& type, RenderCommandBuffer::Family const& family,
                                                                     std::uint32_t const& count)
    : RenderCommandBuffer{type, family, count}
{

    VulkanInstance& vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const  vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();
    std::shared_ptr<VulkanPhysicalDevice> const vulkan_physical_device =
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice();

    // Resize the command buffers using the count variable
    vulkan_command_buffers_.resize(count);

    // Loop through the Command Buffers, setting the queue family index based on the type of family and create command pool with the created info.
    for (VulkanCommandBuffer& vulkan_command_buffer : vulkan_command_buffers_)
    {
        VkCommandPoolCreateInfo vk_command_pool_create_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        vk_command_pool_create_info.pNext = VK_NULL_HANDLE;
        vk_command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        switch (family)
        {
        case Family::_PRESENT_:
            vk_command_pool_create_info.queueFamilyIndex = static_cast<std::uint32_t>(vulkan_physical_device->getQueueFamilyIndices().present);
            break;
        case Family::_GRAPHIC_:
            vk_command_pool_create_info.queueFamilyIndex = static_cast<std::uint32_t>(vulkan_physical_device->getQueueFamilyIndices().graphics);
            break;
        case Family::_TRANSFER_:
            vk_command_pool_create_info.queueFamilyIndex = static_cast<std::uint32_t>(vulkan_physical_device->getQueueFamilyIndices().transfer);
            break;
        case Family::_COMPUTE_:
            vk_command_pool_create_info.queueFamilyIndex = static_cast<std::uint32_t>(vulkan_physical_device->getQueueFamilyIndices().compute);
            break;
        }

        VK_CHECK_RESULT(vkCreateCommandPool(vk_logical_device, &vk_command_pool_create_info, vulkan_instance.allocation_callbaks,
                                            &vulkan_command_buffer.vk_command_pool),
                        "Failed to create command pool!");
        vk_utils::setDebugObjectName<VK_OBJECT_TYPE_COMMAND_POOL>(vulkan_instance.instance, "Vulkan command pool", vk_logical_device,
                                                                  vulkan_command_buffer.vk_command_pool);

        VkCommandBufferAllocateInfo vk_command_buffer_allocate_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        vk_command_buffer_allocate_info.pNext       = VK_NULL_HANDLE;
        vk_command_buffer_allocate_info.commandPool = vulkan_command_buffer.vk_command_pool;
        vk_command_buffer_allocate_info.level       = (type == Type::_PRIMARY_) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        vk_command_buffer_allocate_info.commandBufferCount = 1U;

        VK_CHECK_RESULT(vkAllocateCommandBuffers(vk_logical_device, &vk_command_buffer_allocate_info, &vulkan_command_buffer.vk_command_buffer),
                        "Failed to allocate command buffers!");
        vk_utils::setDebugObjectName<VK_OBJECT_TYPE_COMMAND_BUFFER>(vulkan_instance.instance, "Vulkan command buffer", vk_logical_device,
                                                                    vulkan_command_buffer.vk_command_buffer);
    }
    // Resize the wait fences using the count variable.
    vk_wait_fences_.resize(count);

    // Loop through the Wait Fences, using the VK_FENCE_CREATE_SIGNALED_BIT flag, create with created info variable and set the debug object name.
    for (VkFence& vk_fence : vk_wait_fences_)
    {
        VkFenceCreateInfo vk_fence_create_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        vk_fence_create_info.pNext = VK_NULL_HANDLE;
        vk_fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VK_CHECK_RESULT(vkCreateFence(vk_logical_device, &vk_fence_create_info, vulkan_instance.allocation_callbaks, &vk_fence),
                        "Failed to create fence!");
        vk_utils::setDebugObjectName<VK_OBJECT_TYPE_FENCE>(vulkan_instance.instance, "Vulkan fence ", vk_logical_device, vk_fence);
    }
}

vshade::render::VulkanRenderCommandBuffer::~VulkanRenderCommandBuffer()
{
    VulkanInstance& vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const  vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();

    RenderContext::instance().enqueDelete(
        [vk_wait_fences = vk_wait_fences_, vulkan_command_buffers = vulkan_command_buffers_, vk_logical_device,
         vulkan_instance](std::uint32_t const frame_index)
        {
            for (std::size_t i{0U}; i < vulkan_command_buffers.size(); ++i)
            {
                vkWaitForFences(vk_logical_device, 1U, &vk_wait_fences[i], VK_TRUE, UINT64_MAX);
                vkFreeCommandBuffers(vk_logical_device, vulkan_command_buffers[i].vk_command_pool, 1U, &vulkan_command_buffers[i].vk_command_buffer);
                vkResetCommandPool(vk_logical_device, vulkan_command_buffers[i].vk_command_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
                vkDestroyCommandPool(vk_logical_device, vulkan_command_buffers[i].vk_command_pool, vulkan_instance.allocation_callbaks);
                vkDestroyFence(vk_logical_device, vk_wait_fences[i], vulkan_instance.allocation_callbaks);
            }
        });
}

void vshade::render::VulkanRenderCommandBuffer::begin(std::uint32_t const index)
{
    VkDevice const vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();

    // Create  commandBufferBeginInfo
    VkCommandBufferBeginInfo vk_command_buffer_begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vk_command_buffer_begin_info.pNext            = VK_NULL_HANDLE;
    vk_command_buffer_begin_info.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vk_command_buffer_begin_info.pInheritanceInfo = VK_NULL_HANDLE;

    // Reset the command pool associated with the buffer
    VK_CHECK_RESULT(vkResetCommandPool(vk_logical_device, vulkan_command_buffers_[index].vk_command_pool, 0U), "Failed to reset command pool!");
    // Begin recording command buffer
    VK_CHECK_RESULT(vkBeginCommandBuffer(vulkan_command_buffers_[index].vk_command_buffer, &vk_command_buffer_begin_info),
                    "Failed to begin recording command buffer!");

    is_in_recorder_stage_[index] = true;
}

void vshade::render::VulkanRenderCommandBuffer::end(std::uint32_t const index)
{
    // End recording of a command buffer
    VK_CHECK_RESULT(vkEndCommandBuffer(vulkan_command_buffers_[index].vk_command_buffer), "Failed to end command buffer!");
    is_recordered_[index]        = true;
    is_in_recorder_stage_[index] = false;
}

void vshade::render::VulkanRenderCommandBuffer::submit(std::uint32_t const index, std::chrono::nanoseconds const timeout)
{
    if (is_recordered_[index])
    {
        std::shared_ptr<VulkanLogicalDevice> const vulkan_logical_device{RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()};

        // Define a VkSubmitInfo struct with the appropriate values set for a submit operation
        VkSubmitInfo vk_submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        vk_submit_info.pNext                = VK_NULL_HANDLE;
        vk_submit_info.waitSemaphoreCount   = 0U;
        vk_submit_info.pWaitSemaphores      = VK_NULL_HANDLE;
        vk_submit_info.pWaitDstStageMask    = VK_NULL_HANDLE;
        vk_submit_info.commandBufferCount   = 1U;
        vk_submit_info.pCommandBuffers      = &vulkan_command_buffers_[index].vk_command_buffer;
        vk_submit_info.signalSemaphoreCount = 0U;
        vk_submit_info.pSignalSemaphores    = VK_NULL_HANDLE;

        // Set the Queue to which the submit operation will be sent depending on queue family defined
        VkQueue vk_queue{getVkQueue(family_)};

        // TIP: Family::Present or Family::Graphic is using directly by swap chain, and it's not covered by mutex!
        VK_CHECK_RESULT(vkResetFences(vulkan_logical_device->getVkDevice(), 1U, &vk_wait_fences_[index]), "Failed to reset fences!");
        // Reset a fence before submitting a command buffer which uses it
        getQueueMutex(family_).lock();

        // Submit a command buffer to a queue for execution
        VK_CHECK_RESULT(vkQueueSubmit(vk_queue, 1U, &vk_submit_info, vk_wait_fences_[index]), "Failed to queue submit!");

        getQueueMutex(family_).unlock();
        // Wait for the fence to signal a completion state for a given timeout value
        VK_CHECK_RESULT(vkWaitForFences(vulkan_logical_device->getVkDevice(), 1U, &vk_wait_fences_[index], VK_TRUE, timeout.count()),
                        "Failed to wait fance !");

        is_recordered_[index] = false;
    }
}

std::mutex& vshade::render::VulkanRenderCommandBuffer::getQueueMutex(Family const family)
{
    return mutexs_.at(toVulkanQueueFamilyIndex(family));
}

std::size_t vshade::render::VulkanRenderCommandBuffer::toVulkanQueueFamilyIndex(Family const family)
{
    std::shared_ptr<VulkanLogicalDevice> const vulkan_logical_device{RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()};

    switch (family_)
    {
    case Family::_PRESENT_:
        return static_cast<std::size_t>(vulkan_logical_device->getPhysicalDevice()->getQueueFamilyIndices().present);
    case Family::_GRAPHIC_:
        return static_cast<std::size_t>(vulkan_logical_device->getPhysicalDevice()->getQueueFamilyIndices().graphics);
    case Family::_TRANSFER_:
        return static_cast<std::size_t>(vulkan_logical_device->getPhysicalDevice()->getQueueFamilyIndices().transfer);
    case Family::_COMPUTE_:
        return static_cast<std::size_t>(vulkan_logical_device->getPhysicalDevice()->getQueueFamilyIndices().compute);
    }
}

VkQueue vshade::render::VulkanRenderCommandBuffer::getVkQueue(Family const family)
{
    std::shared_ptr<VulkanLogicalDevice> const vulkan_logical_device{RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()};

    switch (family_)
    {
    case Family::_PRESENT_:
        return vulkan_logical_device->getVulkanQueus().present;
    case Family::_GRAPHIC_:
        return vulkan_logical_device->getVulkanQueus().graphic;
    case Family::_TRANSFER_:
        return vulkan_logical_device->getVulkanQueus().transfer;
    case Family::_COMPUTE_:
        return vulkan_logical_device->getVulkanQueus().compute;
    default:
        return VK_NULL_HANDLE;
    }
}
