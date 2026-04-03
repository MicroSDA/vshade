#include "engine/platforms/render/vulkan/vulkan_frame_buffer.h"
#include <engine/core/render/render.h>

vshade::render::VulkanFrameBuffer::VulkanFrameBuffer(Specification const& specification) : FrameBuffer(specification)
{
    invalidate(specification, nullptr);
}

vshade::render::VulkanFrameBuffer::VulkanFrameBuffer(Specification const& specification, std::vector<std::shared_ptr<Image2D>> const& images)
    : FrameBuffer(specification, images)
{
    invalidate(specification, &images);
}

void vshade::render::VulkanFrameBuffer::invalidate(Specification const& specification, std::vector<std::shared_ptr<Image2D>> const* images)
{
    specification_ = specification;

    color_texture_attachments_.clear(), depth_texture_attachments_.clear();
    vk_color_attachments_views_.clear(), vk_depth_attachments_views_.clear();
    vk_rendering_color_attachment_infos_.clear(), vk_rendering_depth_attachment_infos_.clear();
    vk_attachment_descriptions_.clear(), vk_clear_attachments_.clear();

    std::uint32_t color_attachment_count{0U}, depth_attachment_count = {0U};

    if (images != nullptr)
    {
        for (std::size_t i{0U}; i < specification_.attachments.texture_attachments.size(); ++i)
        {
            // if it is not a depth format attachment
            if (!vk_utils::isDepthFormat(specification_.attachments.texture_attachments[i].format) &&
                !vk_utils::isDepthStencilFormat(specification_.attachments.texture_attachments[i].format))
            {
                ++color_attachment_count;
                color_texture_attachments_.emplace_back(Texture2D::createExplicit((*images)[i]));
                vk_color_attachments_views_.emplace_back(color_texture_attachments_.back()->getImage()->as<VulkanImage2D>().getVkView());
            }
            else // if it is a depth format attachment
            {
                ++depth_attachment_count;
                depth_texture_attachments_.emplace_back(Texture2D::createExplicit((*images)[i]));
                vk_depth_attachments_views_.emplace_back(depth_texture_attachments_.back()->getImage()->as<VulkanImage2D>().getVkView());
            }
        }
    }
    else
    {
        for (std::size_t i{0U}; i < specification_.attachments.texture_attachments.size(); ++i)
        {
            specification_.attachments.texture_attachments[i].width  = specification_.width;
            specification_.attachments.texture_attachments[i].height = specification_.height;
            specification_.attachments.texture_attachments[i].usage  = Image::Usage::_ATTACHMENT_;
            // if it is not a depth format attachment
            if (!vk_utils::isDepthFormat(specification_.attachments.texture_attachments[i].format) &&
                !vk_utils::isDepthStencilFormat(specification_.attachments.texture_attachments[i].format))
            {

                ++color_attachment_count;
                color_texture_attachments_.emplace_back(
                    Texture2D::createExplicit(render::Image2D::create(specification_.attachments.texture_attachments[i])));
                vk_color_attachments_views_.emplace_back(color_texture_attachments_.back()->getImage()->as<VulkanImage2D>().getVkView());
            }
            else // if it is a depth format attachment
            {
                ++depth_attachment_count;
                depth_texture_attachments_.emplace_back(
                    Texture2D::createExplicit(render::Image2D::create(specification_.attachments.texture_attachments[i])));
                vk_depth_attachments_views_.emplace_back(depth_texture_attachments_.back()->getImage()->as<VulkanImage2D>().getVkView());
            }
        }
    }

    vk_rendering_color_attachment_infos_.resize(color_attachment_count);
    vk_rendering_depth_attachment_infos_.resize(depth_attachment_count);
    vk_clear_attachments_.resize(color_attachment_count); // Depth vill be added later

    for (std::size_t i{0U}; i < vk_rendering_color_attachment_infos_.size(); ++i)
    {
        VkRenderingAttachmentInfo& vk_rendering_attachment_info = vk_rendering_color_attachment_infos_.at(i);

        vk_rendering_attachment_info.sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        vk_rendering_attachment_info.pNext              = VK_NULL_HANDLE;
        vk_rendering_attachment_info.imageView          = vk_color_attachments_views_.at(i);
        vk_rendering_attachment_info.imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        vk_rendering_attachment_info.resolveMode        = VK_RESOLVE_MODE_NONE;
        vk_rendering_attachment_info.resolveImageView   = VK_NULL_HANDLE;
        vk_rendering_attachment_info.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        vk_rendering_attachment_info.loadOp             = VK_ATTACHMENT_LOAD_OP_LOAD;
        vk_rendering_attachment_info.storeOp            = VK_ATTACHMENT_STORE_OP_STORE;

        std::memcpy(vk_rendering_attachment_info.clearValue.color.float32, &specification_.clear_color, sizeof(float) * 4U);

        VkClearAttachment& vk_clear_attachment = vk_clear_attachments_.at(i);
        vk_clear_attachment.aspectMask         = VK_IMAGE_ASPECT_COLOR_BIT;
        vk_clear_attachment.colorAttachment    = static_cast<std::uint32_t>(i);
        vk_clear_attachment.clearValue         = vk_rendering_color_attachment_infos_.at(i).clearValue;
    }

    for (std::size_t i{0U}; i < vk_rendering_depth_attachment_infos_.size(); ++i)
    {
        VkRenderingAttachmentInfo& vk_rendering_attachment_info = vk_rendering_depth_attachment_infos_.at(i);

        vk_rendering_attachment_info.sType     = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        vk_rendering_attachment_info.pNext     = VK_NULL_HANDLE;
        vk_rendering_attachment_info.imageView = vk_depth_attachments_views_[i];

        vk_rendering_attachment_info.imageLayout = (!vk_utils::isDepthFormat(depth_texture_attachments_.at(i)->getImage()->getSpecification().format))
                                                       ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
                                                       : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        vk_rendering_attachment_info.resolveMode      = VK_RESOLVE_MODE_NONE;
        vk_rendering_attachment_info.resolveImageView = VK_NULL_HANDLE;

        vk_rendering_attachment_info.resolveImageLayout =
            (!vk_utils::isDepthFormat(depth_texture_attachments_.at(i)->getImage()->getSpecification().format))
                ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
                : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        vk_rendering_attachment_info.loadOp                  = VK_ATTACHMENT_LOAD_OP_LOAD;
        vk_rendering_attachment_info.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
        vk_rendering_attachment_info.clearValue.depthStencil = {specification_.depth_clear_value, 1};

        // Depth should be always at the end of array
        VkClearAttachment& vk_clear_attachment{vk_clear_attachments_.emplace_back()};
        vk_clear_attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        vk_clear_attachment.colorAttachment =
            static_cast<std::uint32_t>(vk_clear_attachments_.size() - 1U); // No one garanti that this will be coorect !
        vk_clear_attachment.clearValue = vk_rendering_attachment_info.clearValue;
    }

    vk_rendering_info_.sType      = VK_STRUCTURE_TYPE_RENDERING_INFO;
    vk_rendering_info_.pNext      = VK_NULL_HANDLE;
    vk_rendering_info_.flags      = 0U;
    vk_rendering_info_.renderArea = {{0U, 0U}, {specification_.width, specification_.height}};
    // If only depth exist get depth as layered, if color attahcment exist take the first attachment as layered !
    vk_rendering_info_.layerCount           = (!color_attachment_count && depth_attachment_count)
                                                  ? depth_texture_attachments_.at(0)->getImage()->getSpecification().layers
                                                  : color_texture_attachments_.at(0)->getImage()->getSpecification().layers;
    vk_rendering_info_.viewMask             = 0U;
    vk_rendering_info_.colorAttachmentCount = color_attachment_count;
    vk_rendering_info_.pColorAttachments    = (color_attachment_count) ? vk_rendering_color_attachment_infos_.data() : VK_NULL_HANDLE;
    vk_rendering_info_.pDepthAttachment     = (color_attachment_count) ? vk_rendering_depth_attachment_infos_.data() : VK_NULL_HANDLE;
    vk_rendering_info_.pStencilAttachment   = VK_NULL_HANDLE;
}

void vshade::render::VulkanFrameBuffer::resize(std::uint32_t const width, std::uint32_t const height)
{
    specification_.width  = width;
    specification_.height = height;
    //------------------------------------------------------------------------
    // Wiat until render thread is free so we can destory vulkan objects safe
    //------------------------------------------------------------------------
    vshade::render::Render::instance().waitUntilRenderDone();
    invalidate(specification_, nullptr);
}

void vshade::render::VulkanFrameBuffer::clearAttachmentsRT(std::shared_ptr<RenderCommandBuffer> const render_command_buffer,
                                                           std::uint32_t const                        frame_index)
{
    VkCommandBuffer vk_command_buffer{render_command_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer(frame_index).vk_command_buffer};

    for (std::size_t i{0U}; i < color_texture_attachments_.size(); ++i)
    {
        VulkanImage2D& vulkan_image{color_texture_attachments_.at(i)->getImage()->as<VulkanImage2D>()};
        VkImageLayout  vk_image_layout{vulkan_image.getVkLayout()};

        VkImageSubresourceRange range{VK_IMAGE_ASPECT_COLOR_BIT};
        range.baseMipLevel   = 0U;
        range.levelCount     = color_texture_attachments_.at(i)->getSpecification().mip_count;
        range.baseArrayLayer = 0U;
        range.layerCount     = color_texture_attachments_.at(i)->getSpecification().layers;

        VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.oldLayout           = vk_image_layout;
        barrier.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image               = vulkan_image.getVkImage();
        barrier.subresourceRange    = range;
        barrier.srcAccessMask       = 0U;
        barrier.dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(vk_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0U, 0U, VK_NULL_HANDLE, 0U,
                             VK_NULL_HANDLE, 1U, &barrier);

        vkCmdClearColorImage(vk_command_buffer, vulkan_image.getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             &vk_rendering_color_attachment_infos_.at(i).clearValue.color, 1U, &range);

        barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout     = vk_image_layout;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        vkCmdPipelineBarrier(vk_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0U, 0U, VK_NULL_HANDLE,
                             0U, VK_NULL_HANDLE, 1U, &barrier);
    }
    for (std::size_t i{0U}; i < depth_texture_attachments_.size(); ++i)
    {
        VulkanImage2D& vulkan_image{depth_texture_attachments_.at(i)->getImage()->as<VulkanImage2D>()};
        VkImageLayout  vk_image_layout{vulkan_image.getVkLayout()};

        VkImageSubresourceRange range{VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT}; // TODO : need to check if it has those flags
        range.baseMipLevel   = 0U;
        range.levelCount     = depth_texture_attachments_.at(i)->getSpecification().mip_count;
        range.baseArrayLayer = 0U;
        range.layerCount     = depth_texture_attachments_.at(i)->getSpecification().layers;

        VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.oldLayout           = vk_image_layout;
        barrier.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image               = vulkan_image.getVkImage();
        barrier.subresourceRange    = range;
        barrier.srcAccessMask       = 0U;
        barrier.dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(vk_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0U, 0U, VK_NULL_HANDLE, 0U,
                             VK_NULL_HANDLE, 1U, &barrier);

        vkCmdClearDepthStencilImage(vk_command_buffer, vulkan_image.getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    &vk_rendering_color_attachment_infos_.at(i).clearValue.depthStencil, 1U, &range);

        barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout     = vk_image_layout;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        vkCmdPipelineBarrier(vk_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0U, 0U, VK_NULL_HANDLE,
                             0U, VK_NULL_HANDLE, 1U, &barrier);
    }
}