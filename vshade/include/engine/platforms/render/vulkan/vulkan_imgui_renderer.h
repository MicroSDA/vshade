#ifndef ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_IMGUI_RENDERER_H
#define ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_IMGUI_RENDERER_H

#include <ankerl/unordered_dense.h>
#include <engine/core/layer/imgui/imgui_renderer.h>
#include <engine/core/layer/layer_resource_manager.h>
#include <engine/platforms/render/vulkan/vulkan_frame_buffer.h>
#include <engine/platforms/render/vulkan/vulkan_render_command_buffer.h>
#include <vulkan/vulkan.h>

namespace vshade
{
namespace render
{
class VSHADE_API VulkanImGuiRenderer : public ImGuiRenderer
{
    friend class utility::CRTPFactory<SceneRenderer>;

public:
    virtual ~VulkanImGuiRenderer();
    VulkanImGuiRenderer(VulkanImGuiRenderer const&)              = delete;
    VulkanImGuiRenderer(VulkanImGuiRenderer&&)                   = delete;
    VulkanImGuiRenderer& operator=(VulkanImGuiRenderer const&) & = delete;
    VulkanImGuiRenderer& operator=(VulkanImGuiRenderer&&) &      = delete;

    virtual void onUpdate(std::shared_ptr<Scene> const scene, time::FrameTimer const& frame_timer, std::uint32_t const frame_index) override;
    virtual void onRenderBegin(time::FrameTimer const& frame_timer, std::uint32_t const frame_index) override;
    virtual void onRender(std::shared_ptr<Scene> const scene, time::FrameTimer const& frame_timer, std::uint32_t const frame_index) override;
    virtual void onRenderEnd(time::FrameTimer const& frame_timer, std::uint32_t const frame_index) override;

    virtual void drawTexture(std::shared_ptr<render::Texture2D> texture, ImVec2 const& size, ImVec4 const& borderColor, float const alpha) override;
    virtual bool drawImageButton(char const* title, std::shared_ptr<render::Texture2D> texture, ImVec2 const& size, ImVec4 const& border_color,
                                 float const alpha) override;

protected:
    explicit VulkanImGuiRenderer(std::shared_ptr<vshade::render::FrameBuffer> const frame_buffer = nullptr);

    VkDescriptorPool                                                          vk_descriptor_pool_{VK_NULL_HANDLE};
    VkDescriptorSet                                                           vk_image_{VK_NULL_HANDLE};
    VkPipeline                                                                vk_pipeline_{VK_NULL_HANDLE};
    std::shared_ptr<RenderCommandBuffer>                                      render_command_buffer_;
    ankerl::unordered_dense::map<std::shared_ptr<Texture2D>, VkDescriptorSet> imgui_texture_descriptors_;
};

} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_IMGUI_RENDERER_H