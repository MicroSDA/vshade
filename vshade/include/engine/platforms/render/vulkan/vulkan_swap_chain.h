#ifndef ENGINE_PLATFORMS_RENDER_VULKAN_SWAP_CAHIN_H
#define ENGINE_PLATFORMS_RENDER_VULKAN_SWAP_CAHIN_H

#include <engine/core/render/swap_chain.h>
#include <engine/platforms/render/vulkan/vulkan_frame_buffer.h>
#include <engine/platforms/render/vulkan/vulkan_image.h>
#include <engine/platforms/render/vulkan/vulkan_render_command_buffer.h>
#include <engine/platforms/render/vulkan/vulkan_render_context.h>
#include <engine/platforms/render/vulkan/vulkan_render_pipeline.h>

namespace vshade
{
namespace render
{
// TODO: set cosntructor protected !!
class VulkanSwapChain final : public SwapChain
{
    // This struct contains Vulkan semaphore objects used for synchronization
    // between the rendering and presentation stages. Semaphores are used to
    // ensure that rendering commands are completed before presenting the
    // rendered image to the swap chain.
    struct VulkanSemaphores
    {
        // Semaphore to signal that the presentation of an image is complete.
        // This semaphore is signaled by the presentation engine when the image
        // has been fully presented and is ready to be used for rendering.
        VkSemaphore present_complete{VK_NULL_HANDLE};
        // Semaphore to signal that rendering commands have been completed.
        // This semaphore is used to synchronize command buffer execution,
        // ensuring that all rendering commands have finished before presenting
        // the image.
        VkSemaphore render_complete{VK_NULL_HANDLE};
    };

public:
    virtual ~VulkanSwapChain();
    VulkanSwapChain(VulkanSwapChain const&)              = delete;
    VulkanSwapChain(VulkanSwapChain&&)                   = delete;
    VulkanSwapChain& operator=(VulkanSwapChain const&) & = delete;
    VulkanSwapChain& operator=(VulkanSwapChain&&) &      = delete;

    virtual std::pair<std::uint32_t, std::uint32_t> createFrame(std::uint32_t const width, std::uint32_t const height,
                                                                std::uint32_t const frames_in_flyght, bool const v_sync) override;
    virtual void                                    createSurface(void* window_native_handle) override;
    virtual void                                    beginFrame(std::uint32_t const frame_index) override;
    virtual void                                    endFrame(std::uint32_t const frame_index) override;
    virtual bool                                    present(std::chrono::nanoseconds const timeout, std::uint32_t const frame_index) override;
    virtual void                                    resize(std::uint32_t const width, std::uint32_t const height) override;
    virtual bool                                    onResizeEvent(std::shared_ptr<event::WindowResize> const event) override;
    virtual void resolveIntoSwapChain(std::shared_ptr<Texture2D const> const texture, std::uint32_t const frame_index,
                                      Texture2D::Filtration const filtration) override;

    VkFormat getVkColorFormat() const
    {
        return vk_color_format_;
    }

    // protected:
    explicit VulkanSwapChain() = default;

private:
    void clearSwapChainImage(VkCommandBuffer vk_command_buffer);
    void getImageFormatAndColorSpace();
    void acquireNextImage(std::chrono::nanoseconds timeout, std::uint32_t frame_index);
    void updtaePresentDeltaTime();

    VkSwapchainKHR                       vk_swap_chain_{VK_NULL_HANDLE};
    VkSurfaceKHR                         vk_surface_{VK_NULL_HANDLE};
    VkFormat                             vk_color_format_{VK_FORMAT_UNDEFINED};
    VkColorSpaceKHR                      vk_color_space_{VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    std::vector<VulkanSemaphores>        vulkan_semaphores_;
    std::uint32_t                        graphics_queue_node_index_{UINT32_MAX};
    std::uint32_t                        current_image_index_{0U};
    std::uint32_t                        image_count_{0U};
    std::shared_ptr<RenderCommandBuffer> swap_chain_command_buffer_;
};
} // namespace render
} // namespace vshade
#endif // ENGINE_PLATFORMS_RENDER_VULKAN_SWAP_CAHIN_H