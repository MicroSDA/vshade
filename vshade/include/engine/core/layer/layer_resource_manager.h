#ifndef ENGINE_CORE_LAYER_LAYER_RESOURCE_MANAGER_H
#define ENGINE_CORE_LAYER_LAYER_RESOURCE_MANAGER_H

#include <ankerl/unordered_dense.h>
#include <engine/core/utility/singleton.h>
#include <string>
#include <typeindex>

namespace vshade
{

struct Resource final
{
    Resource(void* pointer) : pointer_{pointer}
    {
    }
    void* pointer_;
};

struct ResourceKey final
{
    std::string     name;
    std::type_index type;

    bool operator==(ResourceKey const& other) const
    {
        return name == other.name && type == other.type;
    }
};
} // namespace vshade

namespace std
{
template <> struct hash<vshade::ResourceKey>
{
    std::size_t operator()(vshade::ResourceKey const& key) const
    {
        return hash<std::string>()(key.name) ^ hash<std::size_t>()(key.type.hash_code());
    }
};
} // namespace std

namespace vshade
{
class LayerResourceManager final : public utility::CRTPSingleton<LayerResourceManager>
{

    friend class utility::CRTPSingleton<LayerResourceManager>;

public:
    virtual ~LayerResourceManager()                                = default;
    LayerResourceManager(LayerResourceManager const&)              = delete;
    LayerResourceManager(LayerResourceManager&&)                   = delete;
    LayerResourceManager& operator=(LayerResourceManager const&) & = delete;
    LayerResourceManager& operator=(LayerResourceManager&&) &      = delete;

public:
    template <typename T> void declare(std::string const& name, T* resource_ptr)
    {
        ResourceKey key{name, std::type_index{typeid(T)}};
        resources_.emplace(key,Resource{resource_ptr});
    }

    template <typename T> T* get(std::string const& name)
    {
        ResourceKey key{name, std::type_index(typeid(T))};
        auto        it = resources_.find(key);
        if (it != resources_.end() && it->first.type == std::type_index{typeid(T)})
        {
            return static_cast<T*>(it->second.pointer_);
        }
        return nullptr;
    }

    template <typename T> bool has(std::string const& name) const
    {
        ResourceKey key{name, std::type_index(typeid(T))};
        return resources_.count(key) > 0U;
    }

private:
    explicit LayerResourceManager() = default;
    std::unordered_map<ResourceKey, Resource> resources_;
};
} // namespace vshade

#endif // ENGINE_CORE_LAYER_LAYER_RESOURCE_MANAGER_H