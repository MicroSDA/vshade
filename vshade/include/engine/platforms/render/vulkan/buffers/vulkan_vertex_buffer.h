#ifndef ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_VERTEX_BUFFER_H
#define ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_VERTEX_BUFFER_H

#include <engine/core/render/buffers/vertex_buffer.h>
#include <engine/platforms/render/vulkan/vk_utils.h>
#include <engine/platforms/render/vulkan/vulkan_render_command_buffer.h>
#include <engine/platforms/render/vulkan/vulkan_render_context.h>
#include <tuple>
#include <vulkan/vulkan.h>

namespace vshade
{
namespace render
{
class VulkanVertexBuffer : public VertexBuffer
{
    friend class utility::CRTPFactory<VertexBuffer>;

public:
    virtual ~VulkanVertexBuffer();
    VulkanVertexBuffer(VulkanVertexBuffer const&)              = delete;
    VulkanVertexBuffer(VulkanVertexBuffer&&)                   = delete;
    VulkanVertexBuffer& operator=(VulkanVertexBuffer const&) & = delete;
    VulkanVertexBuffer& operator=(VulkanVertexBuffer&&) &      = delete;

    virtual void setData(std::uint32_t const vertex_size, std::uint32_t const vertices_count, void const* data_ptr, std::uint32_t const frame_index,
                         std::uint32_t const offset) override;
    virtual void resize(std::uint32_t const vertex_size, std::uint32_t const vertices_count) override;
    virtual void bind(std::shared_ptr<RenderCommandBuffer> redner_comand_buffer, std::uint32_t frame_index, std::uint32_t binding,
                      std::uint32_t offset) const override;
    virtual std::uint32_t getVerticesCount() const override;

private:
    explicit VulkanVertexBuffer(BufferUsage const usage, std::uint32_t const vertex_size, std::uint32_t const vertices_count,
                                std::uint32_t const count, std::uint32_t const resize_threshold);

    std::tuple<VkBuffer, VkDeviceMemory> createVkBuffer(VulkanInstance vulkan_instance, VkDevice vk_logical_device,
                                                        VkBufferCreateInfo const* pCreateInfo, VkMemoryPropertyFlags const& properties,
                                                        VkPhysicalDevice vk_physical_device);
    void invalidate(BufferUsage const usage, std::uint32_t const vertex_size, std::uint32_t const vertices_count, std::uint32_t const count,
                    std::uint32_t const resize_threshold);

    std::tuple<VkBuffer, VkDeviceMemory> vk_buffer_and_memory_{VK_NULL_HANDLE, VK_NULL_HANDLE};
    std::tuple<VkBuffer, VkDeviceMemory> vk_stagin_buffer_and_memory_{VK_NULL_HANDLE, VK_NULL_HANDLE};
};
} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_VERTEX_BUFFER_H