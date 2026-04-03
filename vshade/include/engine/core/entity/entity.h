#ifndef ENGINE_CORE_ENTITY_ENTITY_H
#define ENGINE_CORE_ENTITY_ENTITY_H

#include <engine/core/entity/entity_manager.h>
#include <engine/core/logs/loger.h>
#include <entt/entt.hpp>

namespace vshade
{
namespace entity
{
class Entity final
{
    friend class EntityManager;

public:
    explicit Entity() = default;
    ~Entity()         = default;

    template <typename T, typename... Args> T& addComponent(Args&&... args)
    {
        if (hasComponent<T>())
        {
            VSHADE_CORE_ERROR("Entity already has the component: '{0}'", typeid(T).name());
        }
        return entity_manager_->getEntities().emplace<T>(entity_handle_, std::forward<Args>(args)...);
    }
    template <typename T> T& getComponent() const
    {
        if (!hasComponent<T>())
        {
            VSHADE_CORE_ERROR("Entity does not have the component: '{0}'", typeid(T).name());
        }
        return entity_manager_->getEntities().get<T>(entity_handle_);
    }
    template <typename T> void removeComponent()
    {
        if (!hasComponent<T>())
        {
            VSHADE_CORE_ERROR("Entity does not have the component: '{0}'", typeid(T).name());
        }
        entity_manager_->getEntities().remove<T>(entity_handle_);
    }
    template <typename T> bool hasComponent() const
    {
        return entity_manager_->getEntities().any_of<T>(entity_handle_);
    }

private:
    explicit Entity(entt::entity const entity_handle, entity::EntityManager* entity_manager);
    entt::entity   entity_handle_{entt::null};
    EntityManager* entity_manager_{nullptr};
};
} // namespace entity

} // namespace vshade

#endif // ENGINE_CORE_ENTITY_ENTITY_H