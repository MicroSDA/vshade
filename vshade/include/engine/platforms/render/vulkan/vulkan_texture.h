#ifndef ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_TEXTURE_H
#define ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_TEXTURE_H

#include <engine/core/render/texture.h>
#include <engine/platforms/render/vulkan/vulkan_image.h>
#include <engine/platforms/render/vulkan/vulkan_render_context.h>
#include <vulkan/vulkan.h>

namespace vshade
{
namespace render
{
class VSHADE_API VulkanTexture2D : public Texture2D
{
    friend class utility::CRTPFactory<Texture2D>;
    friend class Texture2D;

public:
    virtual ~VulkanTexture2D();
    VulkanTexture2D(VulkanTexture2D const&)              = delete;
    VulkanTexture2D(VulkanTexture2D&&)                   = delete;
    VulkanTexture2D& operator=(VulkanTexture2D const&) & = delete;
    VulkanTexture2D& operator=(VulkanTexture2D&&) &      = delete;

    VkDescriptorImageInfo getDescriptorImageInfo() const;
    VkDescriptorImageInfo getDescriptorImageInfoMip(std::uint32_t const mip = 0U) const;
    VkDescriptorImageInfo getDescriptorImageInfoLayer(std::uint32_t const layer = 0U) const;
    VkSampler             getVkSampler() const
    {
        return vk_sampler_;
    }

protected:
    explicit VulkanTexture2D(std::shared_ptr<Image2D> image);

private:
    void                  invalidate();
    VkSampler             vk_sampler_{VK_NULL_HANDLE};
    VkDescriptorImageInfo vk_descriptor_image_info_;
};
} // namespace render
} // namespace vshade

#endif // ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_TEXTURE_H