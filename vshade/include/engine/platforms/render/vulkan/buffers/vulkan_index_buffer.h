#ifndef ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_INDEX_BUFFER_H
#define ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_INDEX_BUFFER_H

#include <engine/core/render/buffers/index_buffer.h>
#include <engine/platforms/render/vulkan/vk_utils.h>
#include <engine/platforms/render/vulkan/vulkan_render_command_buffer.h>
#include <engine/platforms/render/vulkan/vulkan_render_context.h>
#include <tuple>
#include <vulkan/vulkan.h>

namespace vshade
{
namespace render
{
class VulkanIndexBuffer : public IndexBuffer
{
    friend class utility::CRTPFactory<IndexBuffer>;

public:
    virtual ~VulkanIndexBuffer();
    VulkanIndexBuffer(VulkanIndexBuffer const&)              = delete;
    VulkanIndexBuffer(VulkanIndexBuffer&&)                   = delete;
    VulkanIndexBuffer& operator=(VulkanIndexBuffer const&) & = delete;
    VulkanIndexBuffer& operator=(VulkanIndexBuffer&&) &      = delete;

    virtual void setData(std::uint32_t const size, void const* data, std::uint32_t const offset) override;
    virtual void resize(std::uint32_t const size) override;
    virtual void bind(std::shared_ptr<RenderCommandBuffer> redner_comand_buffer, std::uint32_t const frame_index, std::uint32_t const binding,
                      std::uint32_t const offset) const override;

private:
    explicit VulkanIndexBuffer(BufferUsage const usage, std::uint32_t const data_size, std::uint32_t const resize_threshold, void const* data);

    std::tuple<VkBuffer, VkDeviceMemory> createVkBuffer(VulkanInstance vulkan_instance, VkDevice vk_logical_device,
                                                        VkBufferCreateInfo const* pCreateInfo, VkMemoryPropertyFlags const& properties,
                                                        VkPhysicalDevice vk_physical_device);
    void                                 invalidate(BufferUsage const usage, std::uint32_t const data_size, std::uint32_t const resize_threshold);

    std::tuple<VkBuffer, VkDeviceMemory> vk_buffer_and_memory_{VK_NULL_HANDLE, VK_NULL_HANDLE};
    std::tuple<VkBuffer, VkDeviceMemory> vk_stagin_buffer_and_memory_{VK_NULL_HANDLE, VK_NULL_HANDLE};
};
} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_INDEX_BUFFER_H