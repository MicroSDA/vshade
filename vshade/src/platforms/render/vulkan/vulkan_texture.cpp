#include "engine/platforms/render/vulkan/vulkan_texture.h"

vshade::render::VulkanTexture2D::VulkanTexture2D(std::shared_ptr<Image2D> image) : Texture2D{image}
{
    invalidate();
}

vshade::render::VulkanTexture2D::~VulkanTexture2D()
{
    VulkanInstance& vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const  vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();

    RenderContext::instance().enqueDelete(
        [vk_sampler = vk_sampler_, vk_logical_device, vulkan_instance](const std::uint32_t frame_index)
        {
            if (vk_sampler != VK_NULL_HANDLE)
            {
                vkDestroySampler(vk_logical_device, vk_sampler, vulkan_instance.allocation_callbaks);
            }
        });
}

void vshade::render::VulkanTexture2D::invalidate()
{
    VulkanInstance& vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const  vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();

    if (vk_sampler_ != VK_NULL_HANDLE)
    {
        vkDestroySampler(vk_logical_device, vk_sampler_, vulkan_instance.allocation_callbaks);
    }

    VkSamplerCreateInfo vk_sampler_create_info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    vk_sampler_create_info.pNext                   = VK_NULL_HANDLE;
    vk_sampler_create_info.flags                   = 0U;
    vk_sampler_create_info.magFilter               = VK_FILTER_MAX_ENUM;
    vk_sampler_create_info.minFilter               = VK_FILTER_MAX_ENUM;
    vk_sampler_create_info.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
    vk_sampler_create_info.addressModeU            = static_cast<VkSamplerAddressMode>(image2d_->getSpecification().clamp);
    vk_sampler_create_info.addressModeV            = vk_sampler_create_info.addressModeU;
    vk_sampler_create_info.addressModeW            = vk_sampler_create_info.addressModeU;
    vk_sampler_create_info.mipLodBias              = 0.f;
    vk_sampler_create_info.anisotropyEnable        = VK_TRUE;
    vk_sampler_create_info.maxAnisotropy           = 16.0f; // ADD TO SOME RENDER CONFIG !!!
    vk_sampler_create_info.compareEnable           = VK_FALSE;
    vk_sampler_create_info.compareOp               = VK_COMPARE_OP_MAX_ENUM;
    vk_sampler_create_info.minLod                  = 0.f;
    vk_sampler_create_info.maxLod                  = VK_LOD_CLAMP_NONE;
    vk_sampler_create_info.borderColor             = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    vk_sampler_create_info.unnormalizedCoordinates = VK_FALSE;

    if (image2d_->getSpecification().format <= render::Image::Format{5})
    {
        vk_sampler_create_info.magFilter  = VK_FILTER_NEAREST;
        vk_sampler_create_info.minFilter  = VK_FILTER_NEAREST;
        vk_sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }
    else
    {
        vk_sampler_create_info.magFilter  = VK_FILTER_LINEAR;
        vk_sampler_create_info.minFilter  = VK_FILTER_LINEAR;
        vk_sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }

    VK_CHECK_RESULT(vkCreateSampler(vk_logical_device, &vk_sampler_create_info, vulkan_instance.allocation_callbaks, &vk_sampler_),
                    "Failed to create image sampler!");
    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_SAMPLER>(vulkan_instance.instance, "Image sampler", vk_logical_device, vk_sampler_);

    /*if (m_Image->GetSpecification().Format == render::Image::Format::DEPTH24STENCIL8 || m_Image->GetSpecification().Format ==
    render::Image::Format::DEPTH32F || m_Image->GetSpecification().Format == render::Image::Format::DEPTH32FSTENCIL8UINT)
        m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    else if (m_Image->GetSpecification().Usage == render::Image::Usage::Texture || m_Image->GetSpecification().Usage ==
    render::Image::Usage::Attachment) m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;*/

    vk_descriptor_image_info_.imageView   = image2d_->as<VulkanImage2D>().getVkView();
    vk_descriptor_image_info_.imageLayout = image2d_->as<VulkanImage2D>().getVkLayout();
    vk_descriptor_image_info_.sampler     = vk_sampler_;
}
VkDescriptorImageInfo vshade::render::VulkanTexture2D::getDescriptorImageInfo() const
{
    return { vk_sampler_, vk_descriptor_image_info_.imageView, vk_descriptor_image_info_.imageLayout };
}

VkDescriptorImageInfo vshade::render::VulkanTexture2D::getDescriptorImageInfoMip(std::uint32_t const mip) const
{
    return { vk_sampler_, image2d_->as<VulkanImage2D>().getViewPerMipLevel(mip), vk_descriptor_image_info_.imageLayout};
}

VkDescriptorImageInfo vshade::render::VulkanTexture2D::getDescriptorImageInfoLayer(std::uint32_t const layer) const
{
    return { vk_sampler_, image2d_->as<VulkanImage2D>().getViewPerLayer(layer), vk_descriptor_image_info_.imageLayout};
}
