#ifndef ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_RENDER_COMMAND_BUFFER_H
#define ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_RENDER_COMMAND_BUFFER_H

#include <engine/core/render/render_command_buffer.h>
#include <engine/platforms/render/vulkan/vk_utils.h>
#include <vector>

namespace vshade
{
namespace render
{
class VSHADE_API VulkanRenderCommandBuffer final : public RenderCommandBuffer
{
    friend class utility::CRTPFactory<RenderCommandBuffer>;

public:
    struct VulkanCommandBuffer
    {
        VkCommandPool   vk_command_pool{VK_NULL_HANDLE};
        VkCommandBuffer vk_command_buffer{VK_NULL_HANDLE};
    };

    virtual ~VulkanRenderCommandBuffer();
    VulkanRenderCommandBuffer(VulkanRenderCommandBuffer const&)              = delete;
    VulkanRenderCommandBuffer(VulkanRenderCommandBuffer&&)                   = delete;
    VulkanRenderCommandBuffer& operator=(VulkanRenderCommandBuffer const&) & = delete;
    VulkanRenderCommandBuffer& operator=(VulkanRenderCommandBuffer&&) &      = delete;

    VulkanCommandBuffer getVulkanCommandBuffer(std::uint32_t const index = 0U)
    {
        if (index >= vulkan_command_buffers_.size())
        {
            VSHADE_CORE_ERROR("Wrong command buffer index!");
        }
        return vulkan_command_buffers_.at(index);
    }
    VkFence getVkFence(std::uint32_t const index = 0U)
    {
        if (index >= vk_wait_fences_.size())
        {
            VSHADE_CORE_ERROR("Wrong fence index!");
        }
        return vk_wait_fences_.at(index);
    }

    virtual void        begin(std::uint32_t const index = 0U) override;
    virtual void        end(std::uint32_t const index = 0U) override;
    virtual void        submit(std::uint32_t const index = 0U, std::chrono::nanoseconds const timeout = std::chrono::nanoseconds::max()) override;
    virtual std::mutex& getQueueMutex(Family const family) override;

private:
    explicit VulkanRenderCommandBuffer(RenderCommandBuffer::Type const& type, RenderCommandBuffer::Family const& family, std::uint32_t const& count);
    std::vector<VulkanCommandBuffer> vulkan_command_buffers_;
    std::vector<VkFence>             vk_wait_fences_;

    std::size_t toVulkanQueueFamilyIndex(Family const family);
    VkQueue     getVkQueue(Family const family);
};
} // namespace render
} // namespace vshade
#endif // ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_RENDER_COMMAND_BUFFER_H