#include "engine/core/render/image.h"
#include <engine/platforms/render/vulkan/vulkan_image.h>

constexpr unsigned char _DUMMY_IMAGE_DATA_1_1_[]{
    0x44, 0x44, 0x53, 0x20, 0x7C, 0x00, 0x00, 0x00, 0x07, 0x10, 0x0A, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x44, 0x58, 0x54, 0x31, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x84, 0xEF, 0x7B, 0xAA, 0xBA, 0xAA, 0xAE};

std::shared_ptr<vshade::render::Image2D> vshade::render::Image2D::Image2D::create(Image& source)
{
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        return utility::CRTPFactory<Image2D>::create<VulkanImage2D>(source);
    }
}
std::shared_ptr<vshade::render::Image2D> vshade::render::Image2D::Image2D::create(Image::Specification const& specification)
{
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        return utility::CRTPFactory<Image2D>::create<VulkanImage2D>(specification);
    }
}
std::shared_ptr<vshade::render::Image2D> vshade::render::Image2D::Image2D::create(Image::Specification const& specification, void const* source)
{
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        return utility::CRTPFactory<Image2D>::create<VulkanImage2D>(specification, source);
    }
}

vshade::render::Image2D::Image2D(Image& source) : image_{source}
{
}

vshade::render::Image2D::Image2D(Image::Specification const& specification) : specification_{specification}
{
}

vshade::render::Image2D::Image2D(Image::Specification const& specification, void const* source) : specification_{specification}
{
}

void vshade::render::Image::serialize(std::ostream& stream) const
{
}
void vshade::render::Image::generateAsDummyImage()
{
    std::stringstream stream;
    stream.write(reinterpret_cast<char const*>(_DUMMY_IMAGE_DATA_1_1_), sizeof(_DUMMY_IMAGE_DATA_1_1_) / sizeof(_DUMMY_IMAGE_DATA_1_1_[0]));
    deserialize(stream);
}

void vshade::render::Image::deserialize(std::istream& stream)
{
    serializer::Serializer::deserialize(stream, image_data_.header);

    if (memcmp(&image_data_.header.magic, "DDS ", 4) == 0) // if magic is DDS
    {
        image_data_.has_alpha_channel = (image_data_.header.flags & 0x00000001) ? true : false;
        // image_data_.is_srgb		= (image_data_.header.flags & 0x00000040)  ? false : true; // Doesnt work, dds doesnt have such flgas
        /*If texture contains compressed RGB data; dwFourCC contains valid data.*/
        if (image_data_.header.dspf.dw_flags == 0x00000004) // DDPF_FOURCC = 0x4
        {
            image_data_.compression = static_cast<Data::DXTCompression>(image_data_.header.dspf.dw_four_cc);
        }

        // Image size
        std::streampos const begin{stream.tellg()};
        stream.seekg(0U, std::ios::end);
        std::streampos const end{stream.tellg()};
        stream.seekg(begin);
        std::uint32_t const image_size = static_cast<std::uint32_t>(end - begin);

        // Create image buffer
        image_data_.data = std::make_shared<std::vector<std::uint8_t>>(image_size);

        image_data_.header.data_size = image_size;
        // Read buffer
        serializer::Serializer::deserialize(stream, *image_data_.data.get()->data(), image_size);
    }
    else
    {
        VSHADE_CORE_WARNING("Wrong image header !: {0}", image_data_.header.magic);
    }
}
