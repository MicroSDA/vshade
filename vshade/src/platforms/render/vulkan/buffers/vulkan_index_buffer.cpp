#include "engine/platforms/render/vulkan/buffers/vulkan_index_buffer.h"

vshade::render::VulkanIndexBuffer::VulkanIndexBuffer(BufferUsage const usage, std::uint32_t const data_size, std::uint32_t const resize_threshold,
                                                     void const* data_ptr)
    : IndexBuffer(usage, data_size, resize_threshold, data_ptr)
{
    invalidate(usage, data_size, resize_threshold);
    setData(data_size, data_ptr, 0U);
}

vshade::render::VulkanIndexBuffer::~VulkanIndexBuffer()
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

void vshade::render::VulkanIndexBuffer::bind(std::shared_ptr<RenderCommandBuffer> redner_comand_buffer, std::uint32_t const frame_index,
                                             std::uint32_t const binding, std::uint32_t const offset) const
{
    VkDeviceSize instance_offset[1U] = {offset};
    vkCmdBindIndexBuffer(redner_comand_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer,
                         std::get<0>(vk_buffer_and_memory_), *instance_offset, VK_INDEX_TYPE_UINT32);
}

void vshade::render::VulkanIndexBuffer::invalidate(BufferUsage const usage, std::uint32_t const size, std::uint32_t const resize_threshold)
{
    VulkanInstance&        vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const         vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();
    VkPhysicalDevice const vk_physical_device =
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getVkDevice();
    std::shared_ptr<VulkanPhysicalDevice> const vulkan_physical_device{
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()};

    data_size_        = size;
    usage_            = usage;
    resize_threshold_ = resize_threshold;

    VkDeviceSize const max_buffer_size{vulkan_physical_device->getPhysicalDeviceProperties4().maxBufferSize};
    if (data_size_ > max_buffer_size)
    {
        VSHADE_CORE_ERROR("Index buffer size {} is biger then VkPhysicalDeviceMaintenance4Properties::maxBufferSize {}", data_size_, max_buffer_size);
    }

    // If buffer exists, destroy it and its memory.
    if (std::get<0>(vk_buffer_and_memory_) != VK_NULL_HANDLE)
    {
        vkFreeMemory(vk_logical_device, std::get<1>(vk_buffer_and_memory_), vulkan_instance.allocation_callbaks);
        vkDestroyBuffer(vk_logical_device, std::get<0>(vk_buffer_and_memory_), vulkan_instance.allocation_callbaks);
    }

    VkBufferCreateInfo vk_buffer_create_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    vk_buffer_create_info.pNext                 = VK_NULL_HANDLE;
    vk_buffer_create_info.flags                 = 0U;
    vk_buffer_create_info.size                  = static_cast<std::uint64_t>(size);
    vk_buffer_create_info.usage                 = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    vk_buffer_create_info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    vk_buffer_create_info.queueFamilyIndexCount = 0U;
    vk_buffer_create_info.pQueueFamilyIndices   = VK_NULL_HANDLE;

    // Use CreateBuffer function to create a buffer for the device also with the memory allocation to it.
    vk_buffer_and_memory_ = createVkBuffer(vulkan_instance, vk_logical_device, &vk_buffer_create_info,
                                           (usage_ == BufferUsage::_GPU_) ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                           vk_physical_device);
}

void vshade::render::VulkanIndexBuffer::setData(std::uint32_t const size, void const* data_ptr, std::uint32_t const offset)
{
    VulkanInstance&        vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const         vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();
    VkPhysicalDevice const vk_physical_device =
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getVkDevice();

    assert(("data_size must not exceed data_size_", size <= data_size_));

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

            VkBufferCopy vk_buffer_copy;
            vk_buffer_copy.srcOffset = 0U;
            vk_buffer_copy.dstOffset = offset;
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
            void* pData;
            vkMapMemory(vk_logical_device, std::get<1>(vk_buffer_and_memory_), offset, size, 0U, &pData);
            memcpy(pData, data_ptr, static_cast<std::size_t>(size));
            vkUnmapMemory(vk_logical_device, std::get<1>(vk_buffer_and_memory_));
        }
    }
}

void vshade::render::VulkanIndexBuffer::resize(std::uint32_t size)
{
    // If size is not null and the current size must be resized according to the resize threshold
    if (size && hasToBeResized(data_size_, size, resize_threshold_))
    {
        // Invalidate the object taking into account the usage, new size and the resize threshold
        invalidate(usage_, size, resize_threshold_);
    }
}

std::tuple<VkBuffer, VkDeviceMemory> vshade::render::VulkanIndexBuffer::createVkBuffer(VulkanInstance vulkan_instance, VkDevice vk_logical_device,
                                                                                       VkBufferCreateInfo const*    pCreateInfo,
                                                                                       VkMemoryPropertyFlags const& properties,
                                                                                       VkPhysicalDevice             vk_physical_device)
{
    VkBuffer vk_buffer{VK_NULL_HANDLE};
    VK_CHECK_RESULT(vkCreateBuffer(vk_logical_device, pCreateInfo, vulkan_instance.allocation_callbaks, &vk_buffer),
                    "Failed to create index buffer!");

    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_BUFFER>(vulkan_instance.instance, "Vulkan index buffer", vk_logical_device, vk_buffer);

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

    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_DEVICE_MEMORY>(vulkan_instance.instance, "Vulkan index buffer memory", vk_logical_device,
                                                               vk_buffer_memory);

    // binds the buffer with the allocated memory
    vkBindBufferMemory(vk_logical_device, vk_buffer, vk_buffer_memory, 0U);

    return {vk_buffer, vk_buffer_memory};
}