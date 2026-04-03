#ifndef ENGINE_CORE_RENDER_DRAWABLE_PRIMITIVES_BOX_H
#define ENGINE_CORE_RENDER_DRAWABLE_PRIMITIVES_BOX_H

#include <engine/core/render/drawable/drawable.h>

namespace vshade
{
namespace render
{
class VSHADE_API Box : public Drawable
{
    friend class utility::CRTPFactory<Drawable>;

public:
    virtual ~Box()               = default;
    Box(Box const&)              = delete;
    Box(Box&&)                   = delete;
    Box& operator=(Box const&) & = delete;
    Box& operator=(Box&&) &      = delete;

protected:
    explicit Box(glm::vec3 const& min_half_ext, glm::vec3 const& max_half_ext);
    glm::vec3 const min_half_ext_;
    glm::vec3 const max_half_ext_;
};
} // namespace render
} // namespace vshade

#endif ENGINE_CORE_RENDER_DRAWABLE_PRIMITIVES_PLANE_H