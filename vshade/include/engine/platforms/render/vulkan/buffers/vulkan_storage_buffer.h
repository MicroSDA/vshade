#ifndef ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_STORAGE_BUFFER_H
#define ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_STORAGE_BUFFER_H

#include <engine/core/render/buffers/storage_buffer.h>
#include <engine/platforms/render/vulkan/vk_utils.h>
#include <engine/platforms/render/vulkan/vulkan_render_command_buffer.h>
#include <engine/platforms/render/vulkan/vulkan_render_context.h>
#include <tuple>
#include <vulkan/vulkan.h>

namespace vshade
{
namespace render
{
class VulkanStorageBuffer : public StorageBuffer
{
    friend class utility::CRTPFactory<StorageBuffer>;

public:
    virtual ~VulkanStorageBuffer();
    VulkanStorageBuffer(VulkanStorageBuffer const&)              = delete;
    VulkanStorageBuffer(VulkanStorageBuffer&&)                   = delete;
    VulkanStorageBuffer& operator=(VulkanStorageBuffer const&) & = delete;
    VulkanStorageBuffer& operator=(VulkanStorageBuffer&&) &      = delete;

    virtual void setData(std::uint32_t const size, void const* data, std::uint32_t const frame_index, std::uint32_t const offset) override;
    virtual void resize(std::uint32_t const size) override;

    std::vector<VkDescriptorBufferInfo> const& getVkDescriptorBufferInfos() const
    {
        return vk_descriptor_buffer_infos_;
    }
    VkDescriptorBufferInfo getVkDescriptorBufferInfo(std::size_t const frame_index) const
    {
        return vk_descriptor_buffer_infos_.at(frame_index);
    }

private:
    explicit VulkanStorageBuffer(BufferUsage const usage, std::uint32_t const binding_index, std::uint32_t const data_size, std::uint32_t const count,
                                 std::uint32_t const resize_threshold);

    std::tuple<VkBuffer, VkDeviceMemory> createVkBuffer(VulkanInstance vulkan_instance, VkDevice vk_logical_device,
                                                        VkBufferCreateInfo const* pCreateInfo, VkMemoryPropertyFlags const& properties,
                                                        VkPhysicalDevice vk_physical_device);
    void          invalidate(BufferUsage const usage, std::uint32_t const binding_index, std::uint32_t const data_size, std::uint32_t const count,
                             std::uint32_t resize_threshold);
    std::uint32_t getBufferMinAlignment(std::size_t const size) const;

    std::tuple<VkBuffer, VkDeviceMemory> vk_buffer_and_memory_{VK_NULL_HANDLE, VK_NULL_HANDLE};
    std::tuple<VkBuffer, VkDeviceMemory> vk_stagin_buffer_and_memory_{VK_NULL_HANDLE, VK_NULL_HANDLE};
    std::uint32_t                        alignment_size_{0U};
    std::vector<VkDescriptorBufferInfo>  vk_descriptor_buffer_infos_;
};
} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_STORAGE_BUFFER_H