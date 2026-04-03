#include "engine/core/render/drawable/drawable.h"

vshade::render::Drawable::Drawable() : material_{Material::create<Material>()}
{
}

void vshade::render::Drawable::addVertex(Vertex const& vertex, std::size_t level_of_details)
{
    getLevelOfDetails(level_of_details).vertices.emplace_back(vertex);
}

void vshade::render::Drawable::addIndex(Index index, std::size_t level_of_details)
{
    getLevelOfDetails(level_of_details).indices.emplace_back(index);
}

void vshade::render::Drawable::addVertices(Vertices const& vertices, std::size_t level_of_details)
{
    std::copy(vertices.begin(), vertices.end(), std::back_inserter(getLevelOfDetails(level_of_details).vertices));
}

void vshade::render::Drawable::addIndices(Indices const& indices, std::size_t level_of_details)
{
    std::copy(indices.begin(), indices.end(), std::back_inserter(getLevelOfDetails(level_of_details).indices));
}

void vshade::render::Drawable::setVertices(Vertices& vertices, std::size_t level_of_details)
{
    getLevelOfDetails(level_of_details).vertices = std::move(vertices);
}

void vshade::render::Drawable::setIndices(Indices& indices, std::size_t level_of_details)
{
    getLevelOfDetails(level_of_details).indices = std::move(indices);
}

// void vshade::render::Drawable::resetVertices(Vertices const& vertices,
//     std::size_t level_of_details)
// {
//     getLevelOfDetails(level_of_details).vertices.clear();
//     std::copy(vertices.begin(), vertices.end(), std::back_inserter(getLevelOfDetails(level_of_details).vertices));
// }

// void vshade::render::Drawable::resetIndices(Indices const& indices,
//     std::size_t level_of_details)
// {
//     getLevelOfDetails(level_of_details).indices.clear();
//     std::copy(indices.begin(), indices.end(), std::back_inserter(getLevelOfDetails(level_of_details).indices));
// }