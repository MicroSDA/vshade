#ifndef EDITOR_RENDERER_RENDERER_H
#define EDITOR_RENDERER_RENDERER_H

#include <engine/core/layer/layer_resource_manager.h>
#include <engine/core/render/scene/scene_renderer.h>

namespace editor
{
class SceneRenderer : public vshade::render::SceneRenderer
{
    friend class vshade::utility::CRTPFactory<vshade::render::SceneRenderer>;

public:
    struct ColorCorrection
    {
        float exposure{1.f};
        float gamma{1.f};
        float contrast{1.f};
        float grain_intensity{0.f};
        float chromatic_aberation_intensity{0.f};
        float time{0.f};
    };

public:
    virtual ~SceneRenderer()                         = default;
    SceneRenderer(SceneRenderer const&)              = delete;
    SceneRenderer(SceneRenderer&&)                   = delete;
    SceneRenderer& operator=(SceneRenderer const&) & = delete;
    SceneRenderer& operator=(SceneRenderer&&) &      = delete;

    virtual void onUpdate(std::shared_ptr<vshade::Scene> const scene, vshade::time::FrameTimer const& frame_timer,
                          std::uint32_t const frame_index) override;
    virtual void onRenderBegin(vshade::time::FrameTimer const& frame_timer, std::uint32_t const frame_index) override;
    virtual void onRender(std::shared_ptr<vshade::Scene> const scene, vshade::time::FrameTimer const& frame_timer,
                          std::uint32_t const frame_index) override;
    virtual void onRenderEnd(vshade::time::FrameTimer const& frame_timer, std::uint32_t const frame_index) override;

private:
    explicit SceneRenderer(std::shared_ptr<vshade::render::FrameBuffer> const frame_buffer = nullptr);

    void gridDrawCallback(std::shared_ptr<vshade::render::RenderPipeline>      render_pipeline,
                          std::shared_ptr<vshade::render::RenderCommandBuffer> render_command_buffer,
                          vshade::render::data::DrawableMaterialMap const&     drawable_material_map,
                          vshade::render::data::SubmittedFrameData const& submitted_frame_fata, std::uint32_t const, bool const is_force_clear);
    void flatDrawCallback(std::shared_ptr<vshade::render::RenderPipeline>      render_pipeline,
                          std::shared_ptr<vshade::render::RenderCommandBuffer> render_command_buffer,
                          vshade::render::data::DrawableMaterialMap const&     drawable_material_map,
                          vshade::render::data::SubmittedFrameData const& submitted_frame_fata, std::uint32_t const, bool const is_force_clear);
    void colorCorrectionCallback(std::shared_ptr<vshade::render::ComputePipeline>     render_pipeline,
                                 std::shared_ptr<vshade::render::RenderCommandBuffer> render_command_buffer, std::uint32_t const);

    std::shared_ptr<vshade::render::RenderCommandBuffer>       render_command_buffer_;
    std::shared_ptr<vshade::render::RenderPipeline>            render_pipeline_;
    std::shared_ptr<vshade::render::RenderPipeline>            grid_render_pipeline_;
    std::shared_ptr<vshade::render::ComputePipeline>           color_correction_pipeline_;
    std::shared_ptr<vshade::render::Camera>                    main_camera_;
    std::vector<std::shared_ptr<vshade::render::VertexBuffer>> vertex_buffers_;
    std::shared_ptr<vshade::render::IndexBuffer>               index_buffer_;
    std::shared_ptr<vshade::render::Plane>                     grid_plane_;
    std::shared_ptr<vshade::render::Material>                  aabb_box_material_;
    ColorCorrection                                            color_correction_;
};
} // namespace editor

#endif // EDITOR_RENDERER_RENDERER_H