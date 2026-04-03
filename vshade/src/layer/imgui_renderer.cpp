#include "engine/core/layer/imgui/imgui_renderer.h"
#include <engine/platforms/render/vulkan/vulkan_imgui_renderer.h>

std::shared_ptr<vshade::render::SceneRenderer> vshade::render::ImGuiRenderer::create(std::shared_ptr<vshade::render::FrameBuffer> const frame_buffer)
{
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        return utility::CRTPFactory<SceneRenderer>::create<VulkanImGuiRenderer>(frame_buffer);
    }
}

vshade::render::ImGuiRenderer::ImGuiRenderer(std::shared_ptr<vshade::render::FrameBuffer> const frame_buffer) : SceneRenderer{frame_buffer}
{
}