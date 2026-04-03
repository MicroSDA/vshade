#include "engine/core/render/texture.h"
#include <engine/platforms/render/vulkan/vulkan_texture.h>

std::shared_ptr<vshade::render::Texture2D> vshade::render::Texture2D::createExplicit(std::shared_ptr<Image2D> image)
{
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        return utility::CRTPFactory<Texture2D>::create<VulkanTexture2D>(image);
    }
}

vshade::render::Texture2D::Texture2D(std::shared_ptr<Image2D> image) : image2d_{image}
{
}
