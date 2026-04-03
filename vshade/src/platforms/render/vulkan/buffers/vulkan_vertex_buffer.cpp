#include "engine/platforms/render/vulkan/buffers/vulkan_vertex_buffer.h"

vshade::render::VulkanVertexBuffer::VulkanVertexBuffer(BufferUsage const usage, std::uint32_t const vertex_size, std::uint32_t const vertices_count,
                                                       std::uint32_t const count, std::uint32_t const resize_threshold)
    : VertexBuffer(usage, vertex_size, vertices_count, count, resize_threshold)
{
    invalidate(usage, vertex_size, vertices_count, count, resize_threshold);
}

vshade::render::VulkanVertexBuffer::~VulkanVertexBuffer()
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

void vshade::render::VulkanVertexBuffer::invalidate(BufferUsage const usage, std::uint32_t const vertex_size, std::uint32_t const vertices_count,
                                                    std::uint32_t const count, std::uint32_t const resize_threshold)
{
    VulkanInstance&        vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const         vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();
    VkPhysicalDevice const vk_physical_device =
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getVkDevice();
    std::shared_ptr<VulkanPhysicalDevice> const vulkan_physical_device{
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()};

    data_size_        = vertex_size * vertices_count;
    vertices_count_   = vertices_count;
    count_            = count;
    vertex_size_      = vertex_size;
    usage_            = usage;
    resize_threshold_ = resize_threshold;

    VkDeviceSize const max_buffer_size{vulkan_physical_device->getPhysicalDeviceProperties4().maxBufferSize};
    if (data_size_ > max_buffer_size)
    {
        VSHADE_CORE_ERROR("Vertex buffer size {} is biger then VkPhysicalDeviceMaintenance4Properties::maxBufferSize {}", data_size_,
                          max_buffer_size);
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
    vk_buffer_create_info.size                  = static_cast<std::uint64_t>(data_size_ * count_);
    vk_buffer_create_info.usage                 = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vk_buffer_create_info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    vk_buffer_create_info.queueFamilyIndexCount = 0U;
    vk_buffer_create_info.pQueueFamilyIndices   = VK_NULL_HANDLE;

    // Use CreateBuffer function to create a buffer for the device also with the memory allocation to it.
    vk_buffer_and_memory_ = createVkBuffer(vulkan_instance, vk_logical_device, &vk_buffer_create_info,
                                           (usage_ == BufferUsage::_GPU_) ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                           vk_physical_device);
}

void vshade::render::VulkanVertexBuffer::setData(std::uint32_t const vertex_size, std::uint32_t const vertices_count, void const* data_ptr,
                                                 std::uint32_t const frame_index, std::uint32_t const offset)
{

    VulkanInstance&        vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const         vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();
    VkPhysicalDevice const vk_physical_device =
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getVkDevice();

    std::uint32_t const data_size{vertex_size * vertices_count};

    assert(("data_size must not exceed data_size_", data_size <= data_size_));

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
            vk_staging_buffer_create_info.size                  = static_cast<std::uint64_t>(data_size);
            vk_staging_buffer_create_info.usage                 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            vk_staging_buffer_create_info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
            vk_staging_buffer_create_info.queueFamilyIndexCount = 0U;
            vk_staging_buffer_create_info.pQueueFamilyIndices   = VK_NULL_HANDLE;

            vk_stagin_buffer_and_memory_ =
                createVkBuffer(vulkan_instance, vk_logical_device, &vk_staging_buffer_create_info,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vk_physical_device);

            // Map the staging device memory to pData to copy the data to it.
            void* p_data;
            vkMapMemory(vk_logical_device, std::get<1>(vk_stagin_buffer_and_memory_), 0U, data_size, 0U, &p_data);
            // std::memset(p_data, 0U, data_size);
            memcpy(p_data, data_ptr, static_cast<std::size_t>(data_size));
            vkUnmapMemory(vk_logical_device, std::get<1>(vk_stagin_buffer_and_memory_));

            VkBufferCopy vk_buffer_copy{};
            vk_buffer_copy.srcOffset = 0U;
            vk_buffer_copy.dstOffset = (frame_index * data_size_) + offset;
            vk_buffer_copy.size      = data_size;

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
            void*               p_data;
            std::uint32_t const new_offset{(frame_index * data_size_) + offset};
            vkMapMemory(vk_logical_device, std::get<1>(vk_buffer_and_memory_), new_offset, data_size, 0U, &p_data);
            // std::memset(p_data, 0U, data_size);
            memcpy(p_data, data_ptr, static_cast<std::size_t>(data_size));
            vkUnmapMemory(vk_logical_device, std::get<1>(vk_buffer_and_memory_));
        }
    }
}

void vshade::render::VulkanVertexBuffer::resize(std::uint32_t const vertex_size, std::uint32_t const vertices_count)
{
    std::uint32_t const data_size{vertex_size * vertices_count};

    // If size is not null and the current size must be resized according to the resize threshold
    if (data_size && hasToBeResized(data_size_, data_size, resize_threshold_))
    {
        // Invalidate the object taking into account the usage, new size and the resize threshold
        invalidate(usage_, vertex_size, vertices_count, count_, resize_threshold_);
    }
}

void vshade::render::VulkanVertexBuffer::bind(std::shared_ptr<RenderCommandBuffer> redner_comand_buffer, std::uint32_t const frame_index,
                                              std::uint32_t const binding, std::uint32_t const offset) const
{
    // Offset with alligment
    VkDeviceSize instance_offset[1] = {(data_size_ * (count_ > 1U) ? frame_index : 0U) & ~0x3};
    vkCmdBindVertexBuffers(redner_comand_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer, binding, 1U,
                           &std::get<0>(vk_buffer_and_memory_), instance_offset);
}

std::tuple<VkBuffer, VkDeviceMemory> vshade::render::VulkanVertexBuffer::createVkBuffer(VulkanInstance vulkan_instance, VkDevice vk_logical_device,
                                                                                        VkBufferCreateInfo const*    pCreateInfo,
                                                                                        VkMemoryPropertyFlags const& properties,
                                                                                        VkPhysicalDevice             vk_physical_device)
{
    VkBuffer vk_buffer{VK_NULL_HANDLE};
    VK_CHECK_RESULT(vkCreateBuffer(vk_logical_device, pCreateInfo, vulkan_instance.allocation_callbaks, &vk_buffer),
                    "Failed to create vertex buffer!");

    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_BUFFER>(vulkan_instance.instance, "Vulkan vertex buffer", vk_logical_device, vk_buffer);

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

    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_DEVICE_MEMORY>(vulkan_instance.instance, "Vulkan vertex buffer memory", vk_logical_device,
                                                               vk_buffer_memory);

    // binds the buffer with the allocated memory
    vkBindBufferMemory(vk_logical_device, vk_buffer, vk_buffer_memory, 0U);

    return {vk_buffer, vk_buffer_memory};
}

std::uint32_t vshade::render::VulkanVertexBuffer::getVerticesCount() const
{
    return vertices_count_;
}