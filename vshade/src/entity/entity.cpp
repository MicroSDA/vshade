#include "engine/core/entity/entity.h"

vshade::entity::Entity::Entity(entt::entity const entity_handle, entity::EntityManager* entity_manager)
    : entity_handle_{entity_handle}, entity_manager_{entity_manager}
{
}