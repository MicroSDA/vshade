#include "engine/core/layer/layer.h"

vshade::Layer::Layer(std::string name, std::shared_ptr<render::SceneRenderer> renderer) : name_{std::move(name)}, renderer_{renderer}
{
    
}
