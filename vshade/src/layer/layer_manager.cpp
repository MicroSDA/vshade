#include "engine/core/layer/layer_manager.h"

void vshade::LayerManager::onLayerAttach(std::shared_ptr<Layer> layer)
{
   layer->onAttach();
}

void vshade::LayerManager::onLayerDetach(std::shared_ptr<Layer> layer)
{
    layer->onDetach();
}