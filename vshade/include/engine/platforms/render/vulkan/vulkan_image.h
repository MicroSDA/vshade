#ifndef ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_IMAGE_2D_H
#define ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_IMAGE_2D_H
#include <engine/core/render/image.h>
#include <engine/platforms/render/vulkan/vk_utils.h>
#include <engine/platforms/render/vulkan/vulkan_render_context.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace vshade
{
namespace render
{
class VulkanImage2D : public Image2D
{
    friend class utility::CRTPFactory<Image2D>;

public:
    virtual ~VulkanImage2D();
    VulkanImage2D(Image2D const&)                    = delete;
    VulkanImage2D(Image2D&&)                         = delete;
    VulkanImage2D& operator=(VulkanImage2D const&) & = delete;
    VulkanImage2D& operator=(VulkanImage2D&&) &      = delete;

    VkImageView getVkView() const
    {
        return vk_image_view_;
    }

    VkImageLayout getVkLayout() const
    {
        return vk_image_layout_;
    }

    VkImageView getViewPerMipLevel(std::uint32_t mip = 0U) const
    {
        return vk_image_views_per_mip_[mip];
    }

    VkImageView getViewPerLayer(std::uint32_t layer = 0U) const
    {
        return vk_image_views_per_layer_[layer];
    }

    VkFormat getVkImageFormat() const
    {
        return vk_image_format_;
    }
    
    VkImage getVkImage()
    {
        return vk_image_;
    }

    VkImageAspectFlags getAspectFlags() const
    {
        return vk_aspect_flags_;
    }

    void layoutTransition(VkCommandBuffer vk_command_buffer, VkImageLayout vk_new_layout, VkAccessFlags vk_src_access_mask,
                          VkAccessFlags vk_dst_access_mask, VkPipelineStageFlags vk_src_stage, VkPipelineStageFlags vk_dst_stage,
                          VkImageAspectFlags vk_aspect_mask, std::uint32_t mip_base_level, std::uint32_t mip_levels, std::uint32_t layer_base_level,
                          std::uint32_t layer_count);

protected:
    explicit VulkanImage2D(Image& source);
    explicit VulkanImage2D(Image::Specification const& specification);
    explicit VulkanImage2D(Image::Specification const& specification, void const* source);

private:
    void invalidate(Image& image);
    void invalidate(Image::Specification const& specification);
    void invalidate(Image::Specification const& specification, void const* source);

    std::tuple<VkBuffer, VkDeviceMemory> createVkBuffer(VulkanInstance vulkan_instance, VkDevice vk_logical_device,
                                                        VkBufferCreateInfo const* pCreateInfo, VkMemoryPropertyFlags const& properties,
                                                        VkPhysicalDevice vk_physical_device);

    VkImageUsageFlags getVkImageUsageFlags();

    std::vector<VkImageView> vk_image_views_per_mip_;
    std::vector<VkImageView> vk_image_views_per_layer_;
    VkImageAspectFlags       vk_aspect_flags_{VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM};
    VkDeviceMemory           vk_image_memory_{VK_NULL_HANDLE};
    VkImageLayout            vk_image_layout_{VK_IMAGE_LAYOUT_UNDEFINED};
    VkDeviceSize             vk_image_gpu_memory_size_{0U};
    VkImageView              vk_image_view_{VK_NULL_HANDLE};
    VkFormat                 vk_image_format_{VK_FORMAT_UNDEFINED};
    VkImage                  vk_image_{VK_NULL_HANDLE};
    bool                     is_depth_{false}, is_depth_stencil_{false}, is_remoute_destorying_{false};
};
} // namespace render
} // namespace vshade

#endif // ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_IMAGE_2D_H