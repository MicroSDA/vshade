#include "engine/core/render/drawable/primitives/box.h"

vshade::render::Box::Box(glm::vec3 const& min_half_ext, glm::vec3 const& max_half_ext) : min_half_ext_{min_half_ext}, max_half_ext_{max_half_ext}
{
    // Lines
    addVertex({{min_half_ext.x, min_half_ext.y, min_half_ext.z}});
    addVertex({{max_half_ext.x, min_half_ext.y, min_half_ext.z}});
    addVertex({{max_half_ext.x, max_half_ext.y, min_half_ext.z}});
    addVertex({{min_half_ext.x, max_half_ext.y, min_half_ext.z}});
    addVertex({{min_half_ext.x, min_half_ext.y, max_half_ext.z}});
    addVertex({{max_half_ext.x, min_half_ext.y, max_half_ext.z}});
    addVertex({{max_half_ext.x, max_half_ext.y, max_half_ext.z}});
    addVertex({{min_half_ext.x, max_half_ext.y, max_half_ext.z}});
    
    addIndices(Indices{0U, 1U, 1U, 2U, 2U, 3U, 3U, 0U, 4U, 5U, 5U, 6U, 6U, 7U, 7U, 4U, 0U, 4U, 1U, 5U, 2U, 6U, 3U, 7U});
}