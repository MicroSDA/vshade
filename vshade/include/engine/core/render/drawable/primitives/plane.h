#ifndef ENGINE_CORE_RENDER_DRAWABLE_PRIMITIVES_PLANE_H
#define ENGINE_CORE_RENDER_DRAWABLE_PRIMITIVES_PLANE_H

#include <engine/core/render/drawable/drawable.h>

namespace vshade
{
namespace render
{
class VSHADE_API Plane : public Drawable
{
    friend class utility::CRTPFactory<Drawable>;

public:
    virtual ~Plane()                 = default;
    Plane(Plane const&)           = delete;
    Plane(Plane&&)                = delete;
    Plane& operator=(Plane const&) & = delete;
    Plane& operator=(Plane&&) &      = delete;

protected:
    explicit Plane(float const width, float const height, std::uint32_t const density);
};
} // namespace render
} // namespace vshade

#endif ENGINE_CORE_RENDER_DRAWABLE_PRIMITIVES_PLANE_H