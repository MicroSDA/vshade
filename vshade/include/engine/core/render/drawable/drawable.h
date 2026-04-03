#ifndef ENGINE_CORE_RENDER_DRAWABLE_DRAWABLE_H
#define ENGINE_CORE_RENDER_DRAWABLE_DRAWABLE_H

#include <engine/config/vshade_api.h>
#include <engine/core/utility/factory.h>

#include <engine/core/render/buffers/index_buffer.h>
#include <engine/core/render/buffers/vertex_buffer.h>
#include <engine/core/render/drawable/material.h>
#include <engine/core/render/drawable/vertex.h>

namespace vshade
{
namespace render
{
class VSHADE_API Drawable : public utility::CRTPFactory<Drawable>
{
    friend class utility::CRTPFactory<Drawable>;

public:
    static constexpr std::size_t _MaxLevelOfDetails_{10U};

    struct LevelOfDitails
    {
        Vertices vertices;
        Indices  indices;
    };

public:
    virtual ~Drawable()                    = default;
    Drawable(Drawable const&)              = delete;
    Drawable(Drawable&&)                   = delete;
    Drawable& operator=(Drawable const&) & = delete;
    Drawable& operator=(Drawable&&) &      = delete;

    void addVertex(Vertex const& vertex, std::size_t level_of_details = 0U);
    void addIndex(Index index, std::size_t level_of_details = 0U);
    void addVertices(Vertices const& vertices, std::size_t level_of_details = 0U);
    void addIndices(Indices const& indices, std::size_t level_of_details = 0U);

    // void resetVertices(Vertices const& vertices, std::size_t level_of_details = 0U);
    // void resetIndices(Indices const& indices, std::size_t level_of_details = 0U);

    void setVertices(Vertices& vertices, std::size_t level_of_details = 0U);
    void setIndices(Indices& indices, std::size_t level_of_details = 0U);

    LevelOfDitails& getLevelOfDetails(std::size_t level)
    {
        return levels_of_details_[level];
    }

    std::array<LevelOfDitails, _MaxLevelOfDetails_> const& getFullGeometry() const
    {
        return levels_of_details_;
    }

    void setMaterial(std::shared_ptr<Material> material)
    {
        material_ = material;
    }

    std::shared_ptr<Material> getMaterial()
    {
        return material_;
    }

    std::shared_ptr<Material> getMaterial() const
    {
        return material_;
    }

protected:
    explicit Drawable();
    std::array<LevelOfDitails, _MaxLevelOfDetails_> levels_of_details_;
    std::shared_ptr<Material>                       material_;
};
} // namespace render
} // namespace vshade

#endif ENGINE_CORE_RENDER_DRAWABLE_DRAWABLE_H