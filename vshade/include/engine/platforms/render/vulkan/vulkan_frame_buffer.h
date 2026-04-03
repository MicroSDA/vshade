#ifndef ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_FRAME_BUFFER_H
#define ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_FRAME_BUFFER_H
#include <engine/core/logs/loger.h>
#include <engine/core/render/buffers/frame_buffer.h>
#include <engine/platforms/render/vulkan/vk_utils.h>
#include <engine/platforms/render/vulkan/vulkan_image.h>
#include <engine/platforms/render/vulkan/vulkan_render_command_buffer.h>
#include <vulkan/vulkan.h>

namespace vshade
{
namespace render
{
class VSHADE_API VulkanFrameBuffer : public FrameBuffer
{
    friend class utility::CRTPFactory<FrameBuffer>;

public:
    virtual ~VulkanFrameBuffer()                             = default;
    VulkanFrameBuffer(Image2D const&)                        = delete;
    VulkanFrameBuffer(Image2D&&)                             = delete;
    VulkanFrameBuffer& operator=(VulkanFrameBuffer const&) & = delete;
    VulkanFrameBuffer& operator=(VulkanFrameBuffer&&) &      = delete;

    virtual void clearAttachmentsRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer, std::uint32_t const frame_index = 0U) override;
    virtual std::shared_ptr<Texture2D> getColorAttachment(std::uint32_t index = 0U) override
    {
        if (index >= color_texture_attachments_.size())
        {
            VSHADE_CORE_ERROR("Wrong color attahcment index!");
        }
        return color_texture_attachments_.at(index);
    }
    virtual std::shared_ptr<Texture2D> getDepthAttachment(std::uint32_t index = 0U) override
    {
        if (index >= depth_texture_attachments_.size())
        {
            VSHADE_CORE_ERROR("Wrong depth attahcment index!");
        }
        return depth_texture_attachments_.at(index);
    }

    virtual void resize(std::uint32_t const width, std::uint32_t const height) override;

    VkRenderingInfo const& getVkRenderingInfo() const
    {
        return vk_rendering_info_;
    }

    std::vector<VkClearAttachment>& getVkClearAttachments()
    {
        return vk_clear_attachments_;
    }

    std::vector<VkRenderingAttachmentInfo>& getColorVkRenderingAttachmentInfo()
    {
        return vk_rendering_color_attachment_infos_;
    }

    std::vector<VkRenderingAttachmentInfo>& getDepthVkRenderingAttachmentInfo()
    {
        return vk_rendering_depth_attachment_infos_;
    }

protected:
    explicit VulkanFrameBuffer(Specification const& specification);
    explicit VulkanFrameBuffer(Specification const& specification, std::vector<std::shared_ptr<Image2D>> const& images);

private:
    void invalidate(Specification const& specification, std::vector<std::shared_ptr<Image2D>> const* images);

    std::vector<std::shared_ptr<Texture2D>> color_texture_attachments_;
    std::vector<VkImageView>                vk_color_attachments_views_;

    std::vector<std::shared_ptr<Texture2D>> depth_texture_attachments_;
    std::vector<VkImageView>                vk_depth_attachments_views_;

    std::vector<VkAttachmentDescription>   vk_attachment_descriptions_;
    std::vector<VkRenderingAttachmentInfo> vk_rendering_color_attachment_infos_;
    std::vector<VkRenderingAttachmentInfo> vk_rendering_depth_attachment_infos_;
    std::vector<VkClearAttachment>         vk_clear_attachments_;

    VkRenderingInfo               vk_rendering_info_{VK_STRUCTURE_TYPE_RENDERING_INFO};
    VkAttachmentSampleCountInfoNV vk_attachment_sample_count_info_nv;
};
} // namespace render
} // namespace vshade

#endif // ENGINE_PLATFORM_RENDER_VULKAN_VULKAN_FRAME_BUFFER_H