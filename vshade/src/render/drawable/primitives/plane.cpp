#include "engine/core/render/drawable/primitives/plane.h"

vshade::render::Plane::Plane(float const width, float const height, std::uint32_t const density)
{
    // Triangle Strip !
    addVertex({ {-1.0,  1.0, 0.0} });
    addVertex({ {-1.0, -1.0, 0.0} });
    addVertex({ { 1.0,  1.0, 0.0} });
    addVertex({ { 1.0, -1.0, 0.0} });

    addIndex(0U); addIndex(1U); addIndex(2U); addIndex(3U);
}