#include "engine/core/render/drawable/2d/sprite.h"

vshade::render::Sprite::Sprite(std::shared_ptr<render::Texture2D> const texture) : texture_{texture}
{
    // Triangle Strip
    addVertex({{-1.0f, 1.0f, 0.0f}, {}, {}, {}, {0.0f, 1.0f}});  // up-left
    addVertex({{-1.0f, -1.0f, 0.0f}, {}, {}, {}, {0.0f, 0.0f}}); // bottom-left
    addVertex({{1.0f, 1.0f, 0.0f}, {}, {}, {}, {1.0f, 1.0f}});   // up-right
    addVertex({{1.0f, -1.0f, 0.0f}, {}, {}, {}, {1.0f, 0.0f}});  // bottom-right

    addIndex(0U);
    addIndex(1U);
    addIndex(2U);
    addIndex(3U);
}