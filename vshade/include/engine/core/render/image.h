#ifndef ENGINE_CORE_RENDER_IMAGE_H
#define ENGINE_CORE_RENDER_IMAGE_H
#include <cstdint>
#include <engine/core/serialalizing/file_manager.h>
#include <engine/core/utility/factory.h>
#include <vector>

namespace vshade
{
namespace render
{
class VSHADE_API Image final
{
    friend class serializer::Serializer;

public:
    enum class Format
    {
        _UNDEFINED_ = 0,
        // Int
        _RED8UN_,
        _BGRA8UN_,
        _RED8UI_,
        _RED16UI_,
        _RED32UI_,
        _DEPTH32FSTENCIL8UINT_,
        // Floating point
        _RED32F_,
        _RG8_,
        _RG16F_,
        _RG32F_,
        _RGB_,
        _RGBA_,
        _BGRA_,
        _RGBA16F_,
        _RGBA32F_,
        _B10R11G11UF_,
        _SRGB_,
        _DEPTH32F_,
        _DEPTH24STENCIL8_,
        _DEPTH_ = _DEPTH24STENCIL8_,

        _MAX_ENUM_ // Set as vulkan ?
    };

    enum class Clamp
    {
        _REPEAT_                 = 0,
        _MIRRORED_REPEAT_        = 1,
        _CLAMP_TO_EDGE_          = 2,
        _CLAMP_TO_BORDER_        = 3,
        _MIRRORED_CLAMP_TO_EDGE_ = 4,
        _MAX_ENUM_               = 0x7FFFFFFF
    };

    enum class Usage
    {
        _NONE_ = 0,
        _TEXTURE_,
        _ATTACHMENT_,
        _STORAGE_
    };

    struct Specification final
    {
        Specification()      = default;
        Format        format = Format::_RGBA_;
        Usage         usage  = Usage::_NONE_;
        Clamp         clamp  = Clamp::_REPEAT_;
        std::uint32_t mip_count{1U};
        std::uint32_t layers{1U};
        std::uint32_t width{1U}, height{1U};
        bool          is_cube_map{false};
    };

    // DDS format
    struct Data final
    {
        std::shared_ptr<std::vector<std::uint8_t>> data;
        bool                                         has_alpha_channel{false}, is_srgb{false};

        enum class DXTCompression : std::uint32_t
        {
            _UNDEFINED_ = 0,
            _DXT1_      = 827611204,
            _DXT3_      = 861165636,
            _DXT5_      = 894720068,
            _DXT10_     = 808540228,
            // Linear unsigned
            _BC5LU_ = 1429553986,
            // Linear signed
            _BC5LS_ = 1395999554
        } compression{DXTCompression::_UNDEFINED_};

        struct Header final
        {
            std::uint32_t magic;
            std::uint32_t data_size;
            std::uint32_t flags;
            std::uint32_t height, width;
            std::uint32_t pitch_or_linear_size;
            std::uint32_t depth;
            std::uint32_t mip_map_count;
            std::uint32_t _reserved1_[11];

            struct DDSPixelFormat final
            {
                std::uint32_t dw_size;
                std::uint32_t dw_flags;
                std::uint32_t dw_four_cc;
                std::uint32_t dw_rgb_bit_count;
                std::uint32_t dw_r_bit_mask, dw_g_bit_mask, dw_b_bit_mask, dw_a_bit_mask;
            } dspf;

            std::uint32_t caps;
            std::uint32_t caps2;
            std::uint32_t caps3;
            std::uint32_t caps4;
            std::uint32_t _reserved2_;
        } header;
    };

public:
    Image()  = default;
    ~Image() = default;
    Data& getData()
    {
        return image_data_;
    }
    Data const& getData() const
    {
        return image_data_;
    }

    void generateAsDummyImage();

protected:
    Data image_data_;

    void serialize(std::ostream& stream) const;
    void deserialize(std::istream& stream);
};

class VSHADE_API Image2D : public utility::CRTPFactory<Image2D>
{
    friend class utility::CRTPFactory<Image2D>;

    // Hide functions
    using utility::CRTPFactory<Image2D>::create;

public:
    virtual ~Image2D()                   = default;
    Image2D(Image2D const&)              = delete;
    Image2D(Image2D&&)                   = delete;
    Image2D& operator=(Image2D const&) & = delete;
    Image2D& operator=(Image2D&&) &      = delete;

    Image::Specification const& getSpecification() const
    {
        return specification_;
    }

    static std::shared_ptr<Image2D> create(Image& source);
    static std::shared_ptr<Image2D> create(Image::Specification const& specification);
    static std::shared_ptr<Image2D> create(Image::Specification const& specification, void const* source);

protected:
    explicit Image2D(Image& source);
    explicit Image2D(Image::Specification const& specification);
    explicit Image2D(Image::Specification const& specification, void const* source);

    Image::Specification specification_;
    Image                image_;
};
} // namespace render

// This function serializes an image on a given output stream
template <> inline void serializer::Serializer::serialize(std::ostream& stream, render::Image const& image)
{
    image.serialize(stream);
}
// This function deserializes an image from a given input stream
template <> inline void serializer::Serializer::deserialize(std::istream& stream, render::Image& image)
{
    image.deserialize(stream);
}
} // namespace vshade

#endif // ENGINE_CORE_RENDER_IMAGE_H