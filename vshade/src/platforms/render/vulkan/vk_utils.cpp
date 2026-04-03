#include "engine/platforms/render/vulkan/vk_utils.h"
#include <engine/platforms/render/vulkan/vulkan_render_context.h>

std::tuple<VkBuffer, VkDeviceMemory> vshade::render::vk_utils::createMemoryBuffer(VkDevice vk_logical_device, VkPhysicalDevice vk_physical_device,
                                                                                  VkInstance                   vk_instance,
                                                                                  VkAllocationCallbacks const* vk_allocator_callback,
                                                                                  VkBufferCreateInfo const*    vk_buffer_create_info,
                                                                                  VkMemoryPropertyFlags const& vk_memory_properties)
{
    VkBuffer       vk_buffer{VK_NULL_HANDLE};
    VkDeviceMemory vk_device_memory{VK_NULL_HANDLE};

    VK_CHECK_RESULT(vkCreateBuffer(vk_logical_device, vk_buffer_create_info, vk_allocator_callback, &vk_buffer), "Failed to create memory buffer!");
    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_BUFFER>(vk_instance, "Memory buffer", vk_logical_device, vk_buffer);

    // retrieves memory requirements for the buffer
    VkMemoryRequirements vk_memory_requirements;
    vkGetBufferMemoryRequirements(vk_logical_device, vk_buffer, &vk_memory_requirements);

    // allocates memory for the buffer
    VkMemoryAllocateInfo vk_memory_allocate_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    vk_memory_allocate_info.pNext          = VK_NULL_HANDLE;
    vk_memory_allocate_info.allocationSize = vk_memory_requirements.size;
    vk_memory_allocate_info.memoryTypeIndex =
        vk_utils::findMemoryType(vk_physical_device, vk_memory_requirements.memoryTypeBits, vk_memory_properties);

    VK_CHECK_RESULT(vkAllocateMemory(vk_logical_device, &vk_memory_allocate_info, vk_allocator_callback, &vk_device_memory),
                    "Failed to allocate memory!");

    // sets a debug name for the buffer memory
    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_DEVICE_MEMORY>(vk_instance, "Buffer device memory", vk_logical_device, vk_device_memory);

    // binds the buffer with the allocated memory
    VK_CHECK_RESULT(vkBindBufferMemory(vk_logical_device, vk_buffer, vk_device_memory, 0U), "Failed to bind buffer memory!");

    // returns the newly created buffer and its assigned memory
    return {vk_buffer, vk_device_memory};
}

std::uint32_t vshade::render::vk_utils::findMemoryType(VkPhysicalDevice vk_physical_device, std::uint32_t const& type_filter,
                                                       VkMemoryPropertyFlags const& vk_memory_properties)
{
    VkPhysicalDeviceMemoryProperties vk_physical_device_memory_properties;
    vkGetPhysicalDeviceMemoryProperties(vk_physical_device, &vk_physical_device_memory_properties);

    for (std::uint32_t i{0U}; i < vk_physical_device_memory_properties.memoryTypeCount; ++i)
    {
        if ((type_filter & (1 << i)) &&
            (vk_physical_device_memory_properties.memoryTypes[i].propertyFlags & vk_memory_properties) == vk_memory_properties)
        {
            return i;
        }
    }

    VSHADE_CORE_ERROR("Unable to find suitable memory type!")
}

bool vshade::render::vk_utils::isDepthFormat(Image::Format const& format)
{
    if (format == Image::Format::_DEPTH32F_)
        return true;

    return false;
}

bool vshade::render::vk_utils::isDepthStencilFormat(Image::Format const& format)
{
    if (format == Image::Format::_DEPTH24STENCIL8_ || format == render::Image::Format::_DEPTH32FSTENCIL8UINT_)
        return true;

    return false;
}

VkFormat vshade::render::vk_utils::toVulkanImageFormat(vshade::render::Image::Format const& format)
{
    switch (format)
    {
    case Image::Format::_RED8UN_:
        return VK_FORMAT_R8_UNORM;
    case Image::Format::_BGRA8UN_:
        return VK_FORMAT_B8G8R8A8_UNORM;
    case Image::Format::_RED8UI_:
        return VK_FORMAT_R8_UINT;
    case Image::Format::_RED16UI_:
        return VK_FORMAT_R16_UINT;
    case Image::Format::_RED32UI_:
        return VK_FORMAT_R32_UINT;
    case Image::Format::_RED32F_:
        return VK_FORMAT_R32_SFLOAT;
    case Image::Format::_RG8_:
        return VK_FORMAT_R8G8_UNORM;
    case Image::Format::_RG16F_:
        return VK_FORMAT_R16G16_SFLOAT;
    case Image::Format::_RG32F_:
        return VK_FORMAT_R32G32_SFLOAT;
    case Image::Format::_RGBA_:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case Image::Format::_BGRA_:
        return VK_FORMAT_B8G8R8A8_UNORM;
    case Image::Format::_RGBA16F_:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case Image::Format::_RGBA32F_:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Image::Format::_B10R11G11UF_:
        return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    case Image::Format::_DEPTH32FSTENCIL8UINT_:
        return VK_FORMAT_D32_SFLOAT_S8_UINT;
    case Image::Format::_DEPTH32F_:
        return VK_FORMAT_D32_SFLOAT;
    case Image::Format::_DEPTH24STENCIL8_:
        return VulkanRenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getDepthForamt();

    default:
        return VK_FORMAT_UNDEFINED;
    }
}

// Without depth
vshade::render::Image::Format vshade::render::vk_utils::fromVulkanImageFormat(VkFormat const& format)
{
    switch (format)
    {
    case VK_FORMAT_R8_UNORM:
        return Image::Format::_RED8UN_;
    case VK_FORMAT_R8_UINT:
        return Image::Format::_RED8UI_;
    case VK_FORMAT_R16_UINT:
        return Image::Format::_RED16UI_;
    case VK_FORMAT_R32_UINT:
        return Image::Format::_RED32UI_;
    case VK_FORMAT_R32_SFLOAT:
        return Image::Format::_RED32F_;
    case VK_FORMAT_R8G8_UNORM:
        return Image::Format::_RG8_;
    case VK_FORMAT_R16G16_SFLOAT:
        return Image::Format::_RG16F_;
    case VK_FORMAT_R32G32_SFLOAT:
        return Image::Format::_RG32F_;
    case VK_FORMAT_R8G8B8A8_UNORM:
        return Image::Format::_RGBA_;
    case VK_FORMAT_B8G8R8A8_UNORM:
        return Image::Format::_BGRA_;
    case VK_FORMAT_R16G16B16A16_SFLOAT:
        return Image::Format::_RGBA16F_;
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        return Image::Format::_RGBA32F_;
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        return Image::Format::_B10R11G11UF_;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return Image::Format::_DEPTH32FSTENCIL8UINT_;
    case VK_FORMAT_D32_SFLOAT:
        return Image::Format::_DEPTH32F_;

    default:
        return Image::Format::_UNDEFINED_;
    }
}