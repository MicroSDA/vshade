#ifndef ENGINE_CORE_LAYER_IMGUI_IMGUI_RENDERER_H
#define ENGINE_CORE_LAYER_IMGUI_IMGUI_RENDERER_H
#include <engine/core/layer/imgui/imgui_threaded_rendering.h>
#include <engine/core/render/scene/scene_renderer.h>
#include <imgui.h>

namespace vshade
{
namespace render
{
class VSHADE_API ImGuiRenderer : public SceneRenderer
{
    friend class utility::CRTPFactory<SceneRenderer>;
    using utility::CRTPFactory<SceneRenderer>::create;

public:
    virtual ~ImGuiRenderer()                         = default;
    ImGuiRenderer(ImGuiRenderer const&)              = delete;
    ImGuiRenderer(ImGuiRenderer&&)                   = delete;
    ImGuiRenderer& operator=(ImGuiRenderer const&) & = delete;
    ImGuiRenderer& operator=(ImGuiRenderer&&) &      = delete;

    static std::shared_ptr<SceneRenderer> create(std::shared_ptr<vshade::render::FrameBuffer> const frame_buffer = nullptr);

    virtual void  drawTexture(std::shared_ptr<render::Texture2D> texture, ImVec2 const& size, ImVec4 const& borderColor, float const alpha = 1.f) = 0;
    virtual bool  drawImageButton(char const* title, std::shared_ptr<render::Texture2D> texture, ImVec2 const& size, ImVec4 const& border_color,
                                  float const alpha)                                                                                              = 0;
    ImGuiContext* getImGuiContext() const
    {
        return imgui_context_;
    }

protected:
    explicit ImGuiRenderer(std::shared_ptr<vshade::render::FrameBuffer> const frame_buffer = nullptr);
    ImGuiContext*                        imgui_context_{nullptr};
};

} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_LAYER_IMGUI_IMGUI_RENDERER_H