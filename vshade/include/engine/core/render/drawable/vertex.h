#ifndef ENGINE_CORE_RENDER_DRAWABLE_VERTEX_H
#define ENGINE_CORE_RENDER_DRAWABLE_VERTEX_H

#include <glm/glm/glm.hpp>
//#include <glm/glm/gtx/matrix_operation.hpp>

namespace vshade
{
namespace render
{
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bi_tangent;
    glm::vec2 uv_coordinates;

    bool operator==(Vertex const& other) const
    {
        return (position == other.position);
    }
    bool operator!=(Vertex const& other) const
    {
        return (position != other.position);
    }
};

using Vertices = std::vector<Vertex>;
using Index    = std::uint32_t;
using Indices  = std::vector<Index>;

static constexpr std::size_t _VertexDataSize_{sizeof(Vertex)};
static constexpr std::size_t _IndexDataSize_{sizeof(Index)};

static std::size_t VERTICES_DATA_SIZE(std::size_t count)
{
    return _VertexDataSize_ * count;
}

static std::size_t INDICES_DATA_SIZE(std::size_t count)
{
    return _IndexDataSize_ * count;
}
} // namespace render

} // namespace vshade

#endif // ENGINE_CORE_RENDER_DRAWABLE_VERTEX_H