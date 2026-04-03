#ifndef ENGINE_CORE_SCENE_SCENE_H
#define ENGINE_CORE_SCENE_SCENE_H

#include <engine/core/entity/entity.h>

namespace vshade
{
class VSHADE_API Scene : public entity::EntityManager
{
    friend class utility::CRTPFactory<EntityManager>;

public:
    virtual ~Scene()                 = default;
    Scene(Scene const&)              = delete;
    Scene(Scene&&)                   = delete;
    Scene& operator=(Scene const&) & = delete;
    Scene& operator=(Scene&&) &      = delete;

protected:
    explicit Scene() = default;
};

} // namespace vshade

#endif // ENGINE_CORE_SCENE_SCENE_H