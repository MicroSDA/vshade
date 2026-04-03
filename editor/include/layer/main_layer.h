#ifndef EDITOR_LAYER_MAIN_LAYER_H
#define EDITOR_LAYER_MAIN_LAYER_H

#include <engine/core/layer/imgui/imgui_layer.h>
#include <glm/gtc/type_ptr.hpp>
#include <renderer/renderer.h>

namespace editor
{

class MainLayer : public vshade::ImGuiLayer
{
    friend class vshade::utility::CRTPFactory<Layer>;

public:
    virtual ~MainLayer()                     = default;
    MainLayer(MainLayer const&)              = delete;
    MainLayer(MainLayer&&)                   = delete;
    MainLayer& operator=(MainLayer const&) & = delete;
    MainLayer& operator=(MainLayer&&) &      = delete;

    virtual void onUpdate(std::shared_ptr<vshade::Scene> scene, vshade::time::FrameTimer const& frame_timer) override;
    virtual void onRender(std::shared_ptr<vshade::Scene> scene, vshade::time::FrameTimer const& frame_timer) override;
    virtual void onEvent(std::shared_ptr<vshade::Scene> scene, std::shared_ptr<vshade::event::Event> const event,
                         vshade::time::FrameTimer const& frame_timer) override;

protected:
    explicit MainLayer();

    float drawTimeSequencer();
    float drawFileMenu();
    void  drawSettings(float const y_offset, float const alpha);
    void  drawPerformanceOverlay(vshade::time::FrameTimer const& frame_timer, float const y_offset, float const alpha);
    float drawCollapseButtons(float const y_offset);
    void  drawFunnyAnimation(vshade::time::FrameTimer const& frame_timer);
    void  updateCamera(vshade::time::FrameTimer const& frame_timer);

private:
    std::shared_ptr<vshade::render::Camera>    main_camera_;
    glm::ivec2                                 frame_size_{2.f, 2.f};
    int                                        render_resoulution_devider_{1};
    float                                      play_speed_{1.f};
    float                                      camera_movement_speed_{0.03f};
    float                                      camera_rotation_speed_{0.15f};
    bool                                       is_setting_opend_{false};
    bool                                       is_performace_opend_{true};
    bool                                       is_file_dialog_open_{false};
};
} // namespace editor

#endif // EDITOR_LAYER_MAIN_LAYER_H