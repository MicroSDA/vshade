#ifndef ENGINE_CORE_SCENE_SCENE_RENDERER_H
#define ENGINE_CORE_SCENE_SCENE_RENDERER_H

#include <engine/config/system.h>
#include <engine/core/events/input.h>
#include <engine/core/render/drawable/primitives/plane.h>
#include <engine/core/render/render.h>
#include <engine/core/scene/scene.h>
#include <engine/core/time/frame_time.h>
#include <engine/core/utility/factory.h>
namespace vshade
{
namespace render
{

class VSHADE_API SceneRenderer : public utility::CRTPFactory<SceneRenderer>
{
    friend class utility::CRTPFactory<SceneRenderer>;

public:
    virtual ~SceneRenderer()                         = default;
    SceneRenderer(SceneRenderer const&)              = delete;
    SceneRenderer(SceneRenderer&&)                   = delete;
    SceneRenderer& operator=(SceneRenderer const&) & = delete;
    SceneRenderer& operator=(SceneRenderer&&) &      = delete;

    virtual void onUpdate(std::shared_ptr<Scene> const scene, time::FrameTimer const& frame_timer, std::uint32_t const frame_index) = 0;
    virtual void onRenderBegin(time::FrameTimer const& frame_timer, std::uint32_t const frame_index)                                = 0;
    virtual void onRender(std::shared_ptr<Scene> const scene, time::FrameTimer const& frame_timer, std::uint32_t const frame_index) = 0;
    virtual void onRenderEnd(time::FrameTimer const& frame_timer, std::uint32_t const frame_index)                                  = 0;

    std::shared_ptr<vshade::render::FrameBuffer> getMainFrameBuffer()
    {
        return main_frame_buffer_;
    }

protected:
    explicit SceneRenderer(std::shared_ptr<FrameBuffer> const frame_buffer = nullptr)
        : main_frame_buffer_{frame_buffer}, is_external_frame_buffer_{frame_buffer ? true : false}
    {
    }
    std::shared_ptr<vshade::render::FrameBuffer> main_frame_buffer_;
    bool const                                   is_external_frame_buffer_;
};
class VSHADE_API MainSceneRenderer : public SceneRenderer
{
    friend class utility::CRTPFactory<SceneRenderer>;

public:
    virtual ~MainSceneRenderer()                             = default;
    MainSceneRenderer(MainSceneRenderer const&)              = delete;
    MainSceneRenderer(MainSceneRenderer&&)                   = delete;
    MainSceneRenderer& operator=(MainSceneRenderer const&) & = delete;
    MainSceneRenderer& operator=(MainSceneRenderer&&) &      = delete;

    virtual void onUpdate(std::shared_ptr<Scene> const scene, time::FrameTimer const& frame_timer, std::uint32_t const frame_index) override;
    virtual void onRenderBegin(time::FrameTimer const& frame_timer, std::uint32_t const frame_index) override;
    virtual void onRender(std::shared_ptr<Scene> const scene, time::FrameTimer const& frame_timer, std::uint32_t const frame_index) override;
    virtual void onRenderEnd(time::FrameTimer const& frame_timer, std::uint32_t const frame_index) override;

    void basicDrawCallback(std::shared_ptr<RenderPipeline> render_pipeline, std::shared_ptr<RenderCommandBuffer> render_command_buffer,
                           data::DrawableMaterialMap const& drawable_material_map, data::SubmittedFrameData const& submitted_frame_fata,
                           std::uint32_t const frame_index, bool const is_force_clear);

protected:
    explicit MainSceneRenderer();
    std::shared_ptr<RenderCommandBuffer> main_render_command_buffer_;
    std::shared_ptr<RenderPipeline>      main_render_pipeline_;
    std::shared_ptr<RenderPipeline>      off_render_pipeline_;
    std::shared_ptr<Camera>              main_camera_;
    std::shared_ptr<Drawable>            drawable_1_;
    std::shared_ptr<Drawable>            drawable_2_;
    std::shared_ptr<Material>            material_[5];
};

} // namespace render

} // namespace vshade
#endif // ENGINE_CORE_SCENE_SCENE_RENDERER_H