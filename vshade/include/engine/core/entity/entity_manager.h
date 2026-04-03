#ifndef ENGINE_CORE_ENTITY_ENTITY_MANAGER_H
#define ENGINE_CORE_ENTITY_ENTITY_MANAGER_H

#include <engine/core/utility/factory.h>
#include <entt/entt.hpp>

namespace vshade
{
namespace entity
{

class Entity;

class VSHADE_API EntityManager : public utility::CRTPFactory<EntityManager>
{
    friend class utility::CRTPFactory<EntityManager>;

public:
    virtual ~EntityManager()                         = default;
    EntityManager(EntityManager const&)              = delete;
    EntityManager(EntityManager&&)                   = delete;
    EntityManager& operator=(EntityManager const&) & = delete;
    EntityManager& operator=(EntityManager&&) &      = delete;

    vshade::entity::Entity createEntity(std::string const& name = "Unnamed entity");
    void                   destroyEntity(Entity& entity);
    bool const             isValid(Entity const& entity) const;
    entt::registry const&  getEntities() const;
    entt::registry&        getEntities();

protected:
    explicit EntityManager() = default;
    entt::registry registry_;
};

} // namespace entity

} // namespace vshade

#endif // ENGINE_CORE_ENTITY_ENTITY_MANAGER_H