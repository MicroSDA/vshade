#include "engine/core/entity/entity_manager.h"
#include <engine/core/entity/entity.h>

vshade::entity::Entity vshade::entity::EntityManager::createEntity(std::string const& name)
{
    Entity entity{registry_.create(), this};
    entity.addComponent<std::string>(name);
    return entity;
}

void vshade::entity::EntityManager::destroyEntity(Entity& entity)
{
    registry_.destroy(entity.entity_handle_);
}

bool const vshade::entity::EntityManager::isValid(Entity const& entity) const
{
    return registry_.valid(entity.entity_handle_);
}

entt::registry const& vshade::entity::EntityManager::getEntities() const
{
   return registry_;
}

entt::registry& vshade::entity::EntityManager::getEntities()
{
   return registry_;
}

