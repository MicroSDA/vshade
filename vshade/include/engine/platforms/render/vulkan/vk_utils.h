#ifndef ENGINE_CORE_PLATFORM_RENDER_VULKAN_VK_UTILS_H
#define ENGINE_CORE_PLATFORM_RENDER_VULKAN_VK_UTILS_H

#include <vulkan/vulkan.h>

#include <engine/core/logs/loger.h>
#include <engine/core/render/image.h>

#include <iostream>

#define VK_CHECK_RESULT(expr, msg)                                                                                                                   \
                                                                                                                                                     \
    if (expr != VK_SUCCESS)                                                                                                                          \
    {                                                                                                                                                \
        VSHADE_CORE_ERROR("Vulkan : {0}, error code : {1}", msg, static_cast<int>(expr))                                                                                                                       \
    }

namespace vshade
{
namespace render
{
namespace vk_utils
{
template <VkObjectType type> void setDebugObjectName(VkInstance vk_instance, std::string const& name, VkDevice vk_device, void const* object_handle)
{

#ifdef _VSHADE_DEBUG_
    VkDebugUtilsObjectNameInfoEXT vk_debug_utils_object_name_info{VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
    vk_debug_utils_object_name_info.pNext        = VK_NULL_HANDLE;
    vk_debug_utils_object_name_info.objectType   = type;
    vk_debug_utils_object_name_info.objectHandle = reinterpret_cast<uint64_t>(object_handle);
    vk_debug_utils_object_name_info.pObjectName  = name.c_str();

    
    auto pfn_vkSetDebugUtilsObjectNameEXT =
        reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(vk_instance, "vkSetDebugUtilsObjectNameEXT"));
        
    if (pfn_vkSetDebugUtilsObjectNameEXT != nullptr)
    {
        VK_CHECK_RESULT(pfn_vkSetDebugUtilsObjectNameEXT(vk_device, &vk_debug_utils_object_name_info), "Failed to set debug utils object '{}'");
    }

#endif
}

std::uint32_t findMemoryType(VkPhysicalDevice vk_physical_device, std::uint32_t const& type_filter,
                             VkMemoryPropertyFlags const& vk_memory_properties);

std::tuple<VkBuffer, VkDeviceMemory> createMemoryBuffer(VkDevice vk_logical_device, VkPhysicalDevice vk_physical_device, VkInstance vk_instance,
                                                        VkAllocationCallbacks const* vk_allocator_callback,
                                                        VkBufferCreateInfo const*    vk_buffer_create_info,
                                                        VkMemoryPropertyFlags const& vk_memory_properties);
template <VkMemoryPropertyFlags flags>
VkDeviceMemory createImageMemory(VkDevice vk_logical_device, VkPhysicalDevice vk_physical_device, VkInstance vk_instance,
                                 VkAllocationCallbacks const* vk_allocator_callback, VkImage vk_image)
{
    VkDeviceMemory vk_image_memory{VK_NULL_HANDLE};

    VkMemoryRequirements vk_memory_requirements;
    vkGetImageMemoryRequirements(vk_logical_device, vk_image, &vk_memory_requirements);

    // Set memory allocation info
    VkMemoryAllocateInfo vk_memory_allocate_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    vk_memory_allocate_info.pNext           = VK_NULL_HANDLE;
    vk_memory_allocate_info.allocationSize  = vk_memory_requirements.size,
    vk_memory_allocate_info.memoryTypeIndex = vk_utils::findMemoryType(vk_physical_device, vk_memory_requirements.memoryTypeBits, flags);

    // Allocate memory
    VK_CHECK_RESULT(vkAllocateMemory(vk_logical_device, &vk_memory_allocate_info, vk_allocator_callback, &vk_image_memory),
                    "Failed to allocate image memory!");

    // Set image memory debug name
    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_DEVICE_MEMORY>(vk_instance, "Image device memory", vk_logical_device, vk_image_memory);

    // Bind image memory
    VK_CHECK_RESULT(vkBindImageMemory(vk_logical_device, vk_image, vk_image_memory, 0U), "Failed to bind image memory!");

    return vk_image_memory;
}

bool          isDepthFormat(Image::Format const& format);
bool          isDepthStencilFormat(Image::Format const& format);
VkFormat      toVulkanImageFormat(Image::Format const& format);
Image::Format fromVulkanImageFormat(VkFormat const& format);
} // namespace vk_utils

} // namespace render

} // namespace vshade

#endif // ENGINE_CORE_PLATFORM_RENDER_VULKAN_VK_UTILS_H
