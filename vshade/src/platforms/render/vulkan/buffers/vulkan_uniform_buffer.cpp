#include "engine/platforms/render/vulkan/buffers/vulkan_uniform_buffer.h"

vshade::render::VulkanUniformBuffer::VulkanUniformBuffer(BufferUsage const usage, std::uint32_t const binding_index, std::uint32_t const data_size,
                                                         std::uint32_t const count, std::uint32_t const resize_threshold)
    : UniformBuffer(usage, binding_index, data_size, count, resize_threshold)
{
    invalidate(usage, binding_index, data_size, count, resize_threshold);
}

vshade::render::VulkanUniformBuffer::~VulkanUniformBuffer()
{
    VulkanInstance& vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const  vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();

    RenderContext::instance().enqueDelete(
        [vk_buffer_and_memory = vk_buffer_and_memory_, vk_stagin_buffer_and_memory = vk_stagin_buffer_and_memory_, vk_logical_device,
         vulkan_instance](std::uint32_t const frame_index)
        {
            if (std::get<0>(vk_buffer_and_memory) != VK_NULL_HANDLE)
            {
                vkFreeMemory(vk_logical_device, std::get<1>(vk_buffer_and_memory), vulkan_instance.allocation_callbaks);
                vkDestroyBuffer(vk_logical_device, std::get<0>(vk_buffer_and_memory), vulkan_instance.allocation_callbaks);
            }

            if (std::get<0>(vk_stagin_buffer_and_memory) != VK_NULL_HANDLE)
            {
                vkFreeMemory(vk_logical_device, std::get<1>(vk_stagin_buffer_and_memory), vulkan_instance.allocation_callbaks);
                vkDestroyBuffer(vk_logical_device, std::get<0>(vk_stagin_buffer_and_memory), vulkan_instance.allocation_callbaks);
            }
        });
}

void vshade::render::VulkanUniformBuffer::invalidate(BufferUsage const usage, std::uint32_t const binding_index, std::uint32_t const data_size,
                                                     std::uint32_t const count, std::uint32_t resize_threshold)
{
    VulkanInstance&        vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const         vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();
    VkPhysicalDevice const vk_physical_device =
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getVkDevice();

    data_size_      = data_size;
    alignment_size_ = getBufferMinAlignment(data_size);
    vk_descriptor_buffer_infos_.resize(count);

    // If buffer exists, destroy it and its memory.
    if (std::get<0>(vk_buffer_and_memory_) != VK_NULL_HANDLE)
    {
        vkFreeMemory(vk_logical_device, std::get<1>(vk_buffer_and_memory_), vulkan_instance.allocation_callbaks);
        vkDestroyBuffer(vk_logical_device, std::get<0>(vk_buffer_and_memory_), vulkan_instance.allocation_callbaks);
    }

    VkBufferCreateInfo vk_buffer_create_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    vk_buffer_create_info.pNext       = VK_NULL_HANDLE;
    vk_buffer_create_info.flags       = 0U;
    vk_buffer_create_info.size        = static_cast<std::uint64_t>(alignment_size_ * count);
    vk_buffer_create_info.usage       = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vk_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vk_buffer_create_info.queueFamilyIndexCount = 0U;
    vk_buffer_create_info.pQueueFamilyIndices   = VK_NULL_HANDLE;

    // Use CreateBuffer function to create a buffer for the device also with the memory allocation to it.
    vk_buffer_and_memory_ = createVkBuffer(vulkan_instance, vk_logical_device, &vk_buffer_create_info,
                                           (usage_ == BufferUsage::_GPU_) ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                           vk_physical_device);

    for (std::size_t i{0U}; i < vk_descriptor_buffer_infos_.size(); ++i)
    {
        vk_descriptor_buffer_infos_[i].buffer = std::get<0>(vk_buffer_and_memory_);
        vk_descriptor_buffer_infos_[i].offset = alignment_size_ * i;
        vk_descriptor_buffer_infos_[i].range  = alignment_size_;
    }
}

void vshade::render::VulkanUniformBuffer::setData(std::uint32_t const size, void const* data_ptr, std::uint32_t const frame_index,
                                                  std::uint32_t const offset)
{
    VulkanInstance&        vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const         vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();
    VkPhysicalDevice const vk_physical_device =
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getVkDevice();

    if (data_ptr != nullptr)
    {
        if (usage_ == BufferUsage::_GPU_)
        {
            // check if the staging buffer is not null
            if (std::get<0>(vk_stagin_buffer_and_memory_) != VK_NULL_HANDLE)
            {
                // free the memory of the staging device and allocation callbacks
                vkFreeMemory(vk_logical_device, std::get<1>(vk_stagin_buffer_and_memory_), vulkan_instance.allocation_callbaks);
                // destroy the staging buffer and allocation callbacks
                vkDestroyBuffer(vk_logical_device, std::get<0>(vk_stagin_buffer_and_memory_), vulkan_instance.allocation_callbaks);
            }

            // Create a buffer with properties for use as a staging buffer.
            VkBufferCreateInfo vk_staging_buffer_create_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            vk_staging_buffer_create_info.pNext                 = VK_NULL_HANDLE;
            vk_staging_buffer_create_info.flags                 = 0U;
            vk_staging_buffer_create_info.size                  = static_cast<std::uint64_t>(size);
            vk_staging_buffer_create_info.usage                 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            vk_staging_buffer_create_info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
            vk_staging_buffer_create_info.queueFamilyIndexCount = 0U;
            vk_staging_buffer_create_info.pQueueFamilyIndices   = VK_NULL_HANDLE;

            vk_stagin_buffer_and_memory_ =
                createVkBuffer(vulkan_instance, vk_logical_device, &vk_staging_buffer_create_info,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vk_physical_device);

            // Map the staging device memory to pData to copy the data to it.
            void* pData;
            vkMapMemory(vk_logical_device, std::get<1>(vk_stagin_buffer_and_memory_), 0U, size, 0U, &pData);
            memcpy(pData, data_ptr, static_cast<std::size_t>(size));
            vkUnmapMemory(vk_logical_device, std::get<1>(vk_stagin_buffer_and_memory_));

            VkBufferCopy vk_buffer_copy{};
            vk_buffer_copy.srcOffset = 0U;
            vk_buffer_copy.dstOffset = (frame_index * alignment_size_) + offset;
            vk_buffer_copy.size      = size;

            std::shared_ptr<RenderCommandBuffer> transfer_command_buffer{
                RenderCommandBuffer::create(RenderCommandBuffer::Type::_PRIMARY_, RenderCommandBuffer::Family::_TRANSFER_)};

            // Copy data from stagin buffer to real one.
            transfer_command_buffer->begin();
            vkCmdCopyBuffer(transfer_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer().vk_command_buffer,
                            std::get<0>(vk_stagin_buffer_and_memory_), std::get<0>(vk_buffer_and_memory_), 1U, &vk_buffer_copy);
            transfer_command_buffer->end();

            transfer_command_buffer->submit();

            // Free the staging memory and destroy the staging buffer.
            vkFreeMemory(vk_logical_device, std::get<1>(vk_stagin_buffer_and_memory_), vulkan_instance.allocation_callbaks);
            vkDestroyBuffer(vk_logical_device, std::get<0>(vk_stagin_buffer_and_memory_), vulkan_instance.allocation_callbaks);

            // Set the staging buffer and device memory to null.
            std::get<0>(vk_stagin_buffer_and_memory_) = VK_NULL_HANDLE;
            std::get<1>(vk_stagin_buffer_and_memory_) = VK_NULL_HANDLE;
        }
        else
        {
            // Map the staging device memory to pData to copy the data to it.
            void*               pData;
            std::uint32_t const new_offset{(frame_index * alignment_size_) + offset};
            vkMapMemory(vk_logical_device, std::get<1>(vk_buffer_and_memory_), new_offset, size, 0U, &pData);
            memcpy(pData, data_ptr, static_cast<std::size_t>(size));
            vkUnmapMemory(vk_logical_device, std::get<1>(vk_buffer_and_memory_));
        }
    }
}

void vshade::render::VulkanUniformBuffer::resize(std::uint32_t const size)
{
    // If size is not null and the current size must be resized according to the resize threshold
    if (size && hasToBeResized(data_size_, size, resize_threshold_))
    {
        // Invalidate the object taking into account the usage, new size and the resize threshold
        invalidate(usage_, binding_index_, size, count_, resize_threshold_);
    }
}

std::tuple<VkBuffer, VkDeviceMemory> vshade::render::VulkanUniformBuffer::createVkBuffer(VulkanInstance vulkan_instance, VkDevice vk_logical_device,
                                                                                         VkBufferCreateInfo const*    pCreateInfo,
                                                                                         VkMemoryPropertyFlags const& properties,
                                                                                         VkPhysicalDevice             vk_physical_device)
{
    VkBuffer vk_buffer{VK_NULL_HANDLE};
    VK_CHECK_RESULT(vkCreateBuffer(vk_logical_device, pCreateInfo, vulkan_instance.allocation_callbaks, &vk_buffer),
                    "Failed to create uniform buffer!");

    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_BUFFER>(vulkan_instance.instance, "Vulkan uniform buffer", vk_logical_device, vk_buffer);

    // retrieves memory requirements for the buffer
    VkMemoryRequirements vk_memory_requirements;
    vkGetBufferMemoryRequirements(vk_logical_device, vk_buffer, &vk_memory_requirements);

    // allocates memory for the buffer
    VkMemoryAllocateInfo vk_memory_allocate_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    vk_memory_allocate_info.pNext           = VK_NULL_HANDLE;
    vk_memory_allocate_info.allocationSize  = vk_memory_requirements.size;
    vk_memory_allocate_info.memoryTypeIndex = vk_utils::findMemoryType(vk_physical_device, vk_memory_requirements.memoryTypeBits, properties);

    VkDeviceMemory vk_buffer_memory{VK_NULL_HANDLE};
    VK_CHECK_RESULT(vkAllocateMemory(vk_logical_device, &vk_memory_allocate_info, vulkan_instance.allocation_callbaks, &vk_buffer_memory),
                    "Failed to allocate memory!");

    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_DEVICE_MEMORY>(vulkan_instance.instance, "Vulkan uniform buffer memory", vk_logical_device,
                                                               vk_buffer_memory);

    // binds the buffer with the allocated memory
    vkBindBufferMemory(vk_logical_device, vk_buffer, vk_buffer_memory, 0U);

    return {vk_buffer, vk_buffer_memory};
}

std::uint32_t vshade::render::VulkanUniformBuffer::getBufferMinAlignment(std::size_t const size) const
{
    std::shared_ptr<VulkanPhysicalDevice> const vulkan_physical_device{
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()};

    std::uint32_t const max_buffer_size{vulkan_physical_device->getPhysicalDeviceProperties().limits.maxUniformBufferRange};
    if (size > max_buffer_size)
    {
        VSHADE_CORE_ERROR("Uniform buffer size {} is biger then VkPhysicalDeviceLimits::maxUniformBufferRange {}", size, max_buffer_size);
    }

    // Calculate required alignment based on minimum device offset alignment
    std::size_t const min_ubo_alignment{vulkan_physical_device->getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment};
    std::size_t       aligned_size{size};

    if (min_ubo_alignment > 0U)
    {
        aligned_size = (aligned_size + min_ubo_alignment - 1U) & ~(min_ubo_alignment - 1U);
    }

    return aligned_size;
}
