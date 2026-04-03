#ifndef ENGINE_CORE_LAYER_LAYER_H
#define ENGINE_CORE_LAYER_LAYER_H

#include <ankerl/unordered_dense.h>
#include <any>
#include <engine/config/system.h>
#include <engine/config/vshade_api.h>
#include <engine/core/events/event.h>
#include <engine/core/events/input.h>
#include <engine/core/layer/layer_resource_manager.h>
#include <engine/core/render/scene/scene_renderer.h>
#include <engine/core/scene/scene.h>
#include <engine/core/time/frame_time.h>
#include <engine/core/utility/factory.h>
#include <engine/core/window/window.h>

namespace vshade
{

class LayerManager;
class Application;

class VSHADE_API Layer : public utility::CRTPFactory<Layer>
{
    friend class utility::CRTPFactory<Layer>;
    friend class LayerManager;
    friend class Application;

public:
    virtual ~Layer()                 = default;
    Layer(Layer const&)              = delete;
    Layer(Layer&&)                   = delete;
    Layer& operator=(Layer const&) & = delete;
    Layer& operator=(Layer&&) &      = delete;

    void setRenderer(std::shared_ptr<render::SceneRenderer> renderer)
    {
        renderer_ = renderer;
    }
    std::shared_ptr<render::SceneRenderer> getRenderer()
    {
        return renderer_;
    }

    void setScene(std::shared_ptr<Scene> scene)
    {
        scene_ = scene;
    }

    std::string const& getName() const
    {
        return name_;
    }

protected:
    explicit Layer(std::string name, std::shared_ptr<render::SceneRenderer> renderer);

    virtual void onAttach()
    {
    }
    virtual void onDetach()
    {
    }
    virtual void onUpdate(std::shared_ptr<Scene> scene, time::FrameTimer const& frame_timer)
    {
    }
    virtual void onRender(std::shared_ptr<Scene> scene, time::FrameTimer const& frame_timer)
    {
    }
    virtual void onEvent(std::shared_ptr<Scene> scene, std::shared_ptr<event::Event> const event, time::FrameTimer const& frame_timer)
    {
    }

    std::shared_ptr<render::SceneRenderer> renderer_;
    std::string                            name_;

private:
    std::shared_ptr<Scene> scene_;
};
} // namespace vshade

#endif // ENGINE_CORE_LAYER_LAYER_H