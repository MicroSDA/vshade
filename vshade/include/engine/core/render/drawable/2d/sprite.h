#ifndef ENGINE_CORE_RENDER_DRAWABLE_2D_SPRITE_H
#define ENGINE_CORE_RENDER_DRAWABLE_2D_SPRITE_H

#include <engine/core/render/drawable/drawable.h>

namespace vshade
{
namespace render
{
class VSHADE_API Sprite : public Drawable
{
    friend class utility::CRTPFactory<Drawable>;

public:
    virtual ~Sprite()                  = default;
    Sprite(Sprite const&)              = delete;
    Sprite(Sprite&&)                   = delete;
    Sprite& operator=(Sprite const&) & = delete;
    Sprite& operator=(Sprite&&) &      = delete;

    std::shared_ptr<render::Texture2D> getTexture()
    {
        return texture_;
    }

protected:
    explicit Sprite(std::shared_ptr<render::Texture2D> const texture);

    std::shared_ptr<render::Texture2D> const texture_;
};
} // namespace render
} // namespace vshade

#endif ENGINE_CORE_RENDER_DRAWABLE_2D_SPRITE_H