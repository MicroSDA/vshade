#ifndef ENGINE_CORE_RENDER_TEXTURE_H
#define ENGINE_CORE_RENDER_TEXTURE_H
#include <engine/core/render/image.h>
#include <engine/core/utility/factory.h>

namespace vshade
{
namespace render
{
class VSHADE_API Texture2D : public utility::CRTPFactory<Texture2D>
{
    friend class utility::CRTPFactory<Texture2D>;
    // Hide functions
    using utility::CRTPFactory<Texture2D>::create;

public:
    enum class Filtration
    {
        _NEAREST_  = 0U,
        _LINEAR_   = 1U,
        _CUBIC_    = 1000015000U,
        _CUBIC_IMG = _CUBIC_
    };

public:
    virtual ~Texture2D()                     = default;
    Texture2D(Texture2D const&)              = delete;
    Texture2D(Texture2D&&)                   = delete;
    Texture2D& operator=(Texture2D const&) & = delete;
    Texture2D& operator=(Texture2D&&) &      = delete;

    // TODO: add crate from asset manager

    Image::Specification const& getSpecification() const
    {
        return image2d_->getSpecification();
    }

    std::shared_ptr<Image2D> getImage()
    {
        return image2d_;
    }

    std::shared_ptr<Image2D> getImage() const
    {
        return image2d_;
    }

    static std::shared_ptr<Texture2D> createExplicit(std::shared_ptr<Image2D> image);

protected:
    explicit Texture2D(std::shared_ptr<Image2D> image);
    std::shared_ptr<Image2D> image2d_;
};
} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_RENDER_TEXTURE_H