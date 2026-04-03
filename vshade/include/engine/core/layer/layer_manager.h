#ifndef ENGINE_CORE_LAYER_LAYER_MANAGER_H
#define ENGINE_CORE_LAYER_LAYER_MANAGER_H

#include <algorithm>
#include <engine/config/vshade_api.h>
#include <engine/core/layer/layer.h>
#include <engine/core/layer/layer_resource_manager.h>
#include <engine/core/scene/scene.h>
#include <engine/core/utility/singleton.h>
#include <vector>

namespace vshade
{
class VSHADE_API LayerManager : public utility::CRTPSingleton<LayerManager>
{
    friend class utility::CRTPSingleton<LayerManager>;

public:
    virtual ~LayerManager()                        = default;
    LayerManager(LayerManager const&)              = delete;
    LayerManager(LayerManager&&)                   = delete;
    LayerManager& operator=(LayerManager const&) & = delete;
    LayerManager& operator=(LayerManager&&) &      = delete;

    void pushLayer(std::shared_ptr<Layer> layer)
    {
        layers_.emplace(layers_.begin() + layer_insert_index_, layer);
        ++layer_insert_index_;
        layer->onAttach();
    }

    void pushOverlay(std::shared_ptr<Layer> layer)
    {
        layers_.emplace_back(layer);
        layer->onAttach();
    }

    void popLayer(std::shared_ptr<Layer> layer)
    {
        auto it = std::find(layers_.begin(), layers_.begin() + layer_insert_index_, layer);
        if (it != layers_.begin() + layer_insert_index_)
        {
            layer->onDetach();
            layers_.erase(it);
            --layer_insert_index_;
        }
    }

    void popOverlay(std::shared_ptr<Layer> layer)
    {
        auto it = std::find(layers_.begin() + layer_insert_index_, layers_.end(), layer);
        if (it != layers_.end())
        {
            layer->onDetach();
            layers_.erase(it);
        }
    }

    std::vector<std::shared_ptr<Layer>>::iterator begin()
    {
        return layers_.begin();
    }

    std::vector<std::shared_ptr<Layer>>::iterator end()
    {
        return layers_.end();
    }

    std::shared_ptr<Scene> getScene(std::shared_ptr<Layer> layer)
    {
        auto it = std::find(layers_.begin(), layers_.end(), layer);
        if (it != layers_.end())
        {
            return layer->scene_;
        }
        return nullptr;
    }

protected:
    explicit LayerManager()
    {
        LayerResourceManager::create<LayerResourceManager>();
    }
    std::vector<std::shared_ptr<Layer>> layers_;
    std::size_t                         layer_insert_index_{0U};
    void                                onLayerAttach(std::shared_ptr<Layer> layer);
    void                                onLayerDetach(std::shared_ptr<Layer> layer);
};
} // namespace vshade

#endif // ENGINE_CORE_LAYER_LAYER_MANAGER_H