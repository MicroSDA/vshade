#include "engine/platforms/render/vulkan/vulkan_image.h"
#include <engine/platforms/render/vulkan/vulkan_render_command_buffer.h>

namespace utils
{
VkComponentMapping getComponetsSwizzle(vshade::render::Image::Format format, VkImageUsageFlags flags)
{
    switch (format)
    {
    case vshade::render::Image::Format::_UNDEFINED_:
        return VkComponentMapping{VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO};

    case vshade::render::Image::Format::_RED8UN_:
    case vshade::render::Image::Format::_RED8UI_:
    case vshade::render::Image::Format::_RED16UI_:
    case vshade::render::Image::Format::_RED32UI_:
    case vshade::render::Image::Format::_RED32F_:
        if (flags & VK_IMAGE_USAGE_STORAGE_BIT)
        {
            return VkComponentMapping{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                      VK_COMPONENT_SWIZZLE_IDENTITY};
        }
        else
        {
            return VkComponentMapping{VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO};
        }
    case vshade::render::Image::Format::_DEPTH32FSTENCIL8UINT_:
    {
        return VkComponentMapping{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                  VK_COMPONENT_SWIZZLE_IDENTITY};
    }
    case vshade::render::Image::Format::_RG8_:
    case vshade::render::Image::Format::_RG16F_:
    case vshade::render::Image::Format::_RG32F_:
        return VkComponentMapping{VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO};

    case vshade::render::Image::Format::_RGB_:
    case vshade::render::Image::Format::_SRGB_:
    case vshade::render::Image::Format::_B10R11G11UF_:
        return VkComponentMapping{VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_ZERO};

    case vshade::render::Image::Format::_RGBA_:
    case vshade::render::Image::Format::_RGBA16F_:
    case vshade::render::Image::Format::_RGBA32F_:
        return VkComponentMapping{VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};

    case vshade::render::Image::Format::_BGRA_:
        return VkComponentMapping{VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_A};

    case vshade::render::Image::Format::_DEPTH32F_:
    case vshade::render::Image::Format::_DEPTH24STENCIL8_:
        return VkComponentMapping{VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};

    default:
        return VkComponentMapping{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                  VK_COMPONENT_SWIZZLE_IDENTITY};
    }
}
} // namespace utils

vshade::render::VulkanImage2D::VulkanImage2D(Image& source) : Image2D(source)
{
    invalidate(source);
}
vshade::render::VulkanImage2D::VulkanImage2D(Image::Specification const& specification) : Image2D(specification)
{
    invalidate(specification);
}
vshade::render::VulkanImage2D::VulkanImage2D(Image::Specification const& specification, void const* source) : Image2D(specification, source)
{
    invalidate(specification, source);
}

vshade::render::VulkanImage2D::~VulkanImage2D()
{
    VulkanInstance& vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const  vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();

    RenderContext::instance().enqueDelete(
        [is_remoute_destorying = is_remoute_destorying_, vk_image_memory = vk_image_memory_, vk_image = vk_image_, vk_image_view = vk_image_view_,
         vk_image_views_per_mip = vk_image_views_per_mip_, vk_image_views_per_layer = vk_image_views_per_layer_, vulkan_instance,
         vk_logical_device](std::uint32_t const frame_index)
        {
            if (!is_remoute_destorying)
            {
                if (vk_image_memory != VK_NULL_HANDLE)
                {
                    vkFreeMemory(vk_logical_device, vk_image_memory, vulkan_instance.allocation_callbaks);
                }
                if (vk_image != VK_NULL_HANDLE)
                {
                    vkDestroyImage(vk_logical_device, vk_image, vulkan_instance.allocation_callbaks);
                }
            }
            for (VkImageView vk_image_view : vk_image_views_per_mip)
            {
                if (vk_image_view != VK_NULL_HANDLE)
                {
                    vkDestroyImageView(vk_logical_device, vk_image_view, vulkan_instance.allocation_callbaks);
                }
            }
            for (VkImageView vk_image_view : vk_image_views_per_layer)
            {
                if (vk_image_view != VK_NULL_HANDLE)
                {
                    vkDestroyImageView(vk_logical_device, vk_image_view, vulkan_instance.allocation_callbaks);
                }
            }
            if (vk_image_view != VK_NULL_HANDLE)
            {
                vkDestroyImageView(vk_logical_device, vk_image_view, vulkan_instance.allocation_callbaks);
            }
        });
}

void vshade::render::VulkanImage2D::invalidate(Image& source)
{
    VulkanInstance&        vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const         vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();
    VkPhysicalDevice const vk_physical_device =
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getVkDevice();

    Image::Data const& image_data{source.getData()};

    specification_.width     = image_data.header.width;
    specification_.height    = image_data.header.height;
    specification_.layers    = 1U;
    specification_.mip_count = image_data.header.mip_map_count;
    specification_.usage     = render::Image::Usage::_TEXTURE_;
    specification_.format    = render::Image::Format::_RGBA_;

    vk_image_layout_  = VK_IMAGE_LAYOUT_UNDEFINED;
    is_depth_         = vk_utils::isDepthFormat(specification_.format);
    is_depth_stencil_ = vk_utils::isDepthStencilFormat(specification_.format);
    vk_image_format_  = vk_utils::toVulkanImageFormat(specification_.format);

    vk_aspect_flags_ = (is_depth_ || is_depth_stencil_) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    std::uint32_t block_size{0U};

    // Set image format and block size based on data compression
    switch (image_data.compression)
    {
    case render::Image::Data::DXTCompression::_DXT1_:
        if (image_data.has_alpha_channel)
        {
            vk_image_format_ = (image_data.is_srgb) ? VK_FORMAT_BC1_RGBA_SRGB_BLOCK : VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        }
        else
        {
            vk_image_format_ = (image_data.is_srgb) ? VK_FORMAT_BC1_RGB_SRGB_BLOCK : VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        }
        block_size = 8U;
        break;
    case render::Image::Data::DXTCompression::_DXT3_:
        vk_image_format_ = (image_data.is_srgb) ? VK_FORMAT_BC2_SRGB_BLOCK : VK_FORMAT_BC2_UNORM_BLOCK;
        block_size       = 16U;
        break;
    case render::Image::Data::DXTCompression::_DXT5_:
        vk_image_format_ = (image_data.is_srgb) ? VK_FORMAT_BC3_SRGB_BLOCK : VK_FORMAT_BC3_UNORM_BLOCK;
        block_size       = 16U;
        break;
        // TODO: Need test this
    case render::Image::Data::DXTCompression::_BC5LU_:
        vk_image_format_ = VK_FORMAT_BC5_UNORM_BLOCK;
        block_size       = 16U;
        break;
    case render::Image::Data::DXTCompression::_BC5LS_:
        vk_image_format_ = VK_FORMAT_BC5_SNORM_BLOCK;
        block_size       = 16U;
        break;
    default:
        VSHADE_CORE_WARNING("Unsupported texture format in '{0}'.", static_cast<std::uint32_t>(image_data.compression));
        return;
    }

    VkImageUsageFlags vk_image_usage_flags{getVkImageUsageFlags()};

    VkImageCreateInfo vk_image_create_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    vk_image_create_info.pNext       = VK_NULL_HANDLE;
    vk_image_create_info.flags       = (specification_.is_cube_map) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : static_cast<VkImageCreateFlags>(0x0);
    vk_image_create_info.imageType   = VK_IMAGE_TYPE_2D;
    vk_image_create_info.format      = vk_image_format_;
    vk_image_create_info.extent      = {specification_.width, specification_.height, 1U};
    vk_image_create_info.mipLevels   = specification_.mip_count;
    vk_image_create_info.arrayLayers = specification_.layers;
    vk_image_create_info.samples     = VK_SAMPLE_COUNT_1_BIT;
    vk_image_create_info.tiling      = VK_IMAGE_TILING_OPTIMAL;
    vk_image_create_info.usage       = vk_image_usage_flags;
    vk_image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO: Take a look !!
    vk_image_create_info.queueFamilyIndexCount = 0U;
    vk_image_create_info.pQueueFamilyIndices   = VK_NULL_HANDLE;
    vk_image_create_info.initialLayout         = vk_image_layout_;

    // Create image
    VK_CHECK_RESULT(vkCreateImage(vk_logical_device, &vk_image_create_info, vulkan_instance.allocation_callbaks, &vk_image_),
                    "Failed to create image!");
    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_IMAGE>(vulkan_instance.instance, "Image", vk_logical_device, vk_image_);

    // Create and bind image memory
    vk_image_memory_ = vk_utils::createImageMemory<VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT>(
        vk_logical_device, vk_physical_device, vulkan_instance.instance, vulkan_instance.allocation_callbaks, vk_image_);

    // Create a buffer with properties for use as a staging buffer.
    VkBufferCreateInfo vk_staging_buffer_create_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    vk_staging_buffer_create_info.pNext                 = VK_NULL_HANDLE;
    vk_staging_buffer_create_info.flags                 = 0U;
    vk_staging_buffer_create_info.size                  = static_cast<std::uint64_t>(image_data.header.data_size);
    vk_staging_buffer_create_info.usage                 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vk_staging_buffer_create_info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    vk_staging_buffer_create_info.queueFamilyIndexCount = 0U;
    vk_staging_buffer_create_info.pQueueFamilyIndices   = VK_NULL_HANDLE;

    std::tuple<VkBuffer, VkDeviceMemory> vk_stagin_buffer_and_memory{
        createVkBuffer(vulkan_instance, vk_logical_device, &vk_staging_buffer_create_info,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vk_physical_device)};

    // Map the staging buffer memory
    void* data;
    vkMapMemory(vk_logical_device, std::get<1>(vk_stagin_buffer_and_memory), 0U, image_data.header.data_size, 0U, &data);
    // Copy the texture data to the staging buffer memory
    memcpy(data, image_data.data->data(), image_data.header.data_size);
    // Unmap the staging buffer memory
    vkUnmapMemory(vk_logical_device, std::get<1>(vk_stagin_buffer_and_memory));

    std::shared_ptr<RenderCommandBuffer> commad_buffer{
        RenderCommandBuffer::create(RenderCommandBuffer::Type::_PRIMARY_, RenderCommandBuffer::Family::_GRAPHIC_)};

    commad_buffer->begin();

    //------------------------------------------------------------------------
    // Set layout as VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL before transfer data
    // from cpu to texture buffer
    //------------------------------------------------------------------------
    layoutTransition(commad_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer().vk_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, vk_aspect_flags_, 0U,
                     specification_.mip_count, 0U, specification_.layers);

    std::vector<VkBufferImageCopy> vk_buffer_image_copy_regions;
    std::uint32_t                  mip_width{image_data.header.width}, mip_height{image_data.header.height}, buffer_offset{0U};

    for (std::uint32_t i{0U}; i < image_data.header.mip_map_count; ++i)
    {
        std::uint32_t const size = ((mip_width + 3) / 4) * ((mip_height + 3) / 4) * block_size;

        VkBufferImageCopy vk_buffer_amage_copy;
        vk_buffer_amage_copy.bufferOffset                    = buffer_offset;
        vk_buffer_amage_copy.bufferRowLength                 = 0U;
        vk_buffer_amage_copy.bufferImageHeight               = 0U;
        vk_buffer_amage_copy.imageSubresource.aspectMask     = vk_aspect_flags_;
        vk_buffer_amage_copy.imageSubresource.baseArrayLayer = 0U;
        vk_buffer_amage_copy.imageSubresource.layerCount     = 1U;
        vk_buffer_amage_copy.imageSubresource.mipLevel       = i;
        vk_buffer_amage_copy.imageOffset                     = {0, 0, 0};
        vk_buffer_amage_copy.imageExtent                     = {mip_width, mip_height, 1U};
        vk_buffer_image_copy_regions.push_back(vk_buffer_amage_copy);

        // Calculate the buffer offset for the next mip level
        buffer_offset += size; // Assuming RGBA8 format
        // Calculate the dimensions for the next mip level
        mip_width  = std::max(1U, mip_width / 2U);
        mip_height = std::max(1U, mip_height / 2U);
    }

    //------------------------------------------------------------------------
    // Copy data from cpu to image buffer
    //------------------------------------------------------------------------
    vkCmdCopyBufferToImage(commad_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer().vk_command_buffer,
                           std::get<0>(vk_stagin_buffer_and_memory), vk_image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<std::uint32_t>(vk_buffer_image_copy_regions.size()), vk_buffer_image_copy_regions.data());

    //------------------------------------------------------------------------
    // Set VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL layout for shader read only
    //------------------------------------------------------------------------
    layoutTransition(commad_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer().vk_command_buffer,
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     VK_ACCESS_TRANSFER_WRITE_BIT,          // srcAccessMask (ждём записи копирования)
                     VK_ACCESS_SHADER_READ_BIT,             // dstAccessMask (семплинг)
                     VK_PIPELINE_STAGE_TRANSFER_BIT,        // srcStage
                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // dstStage
                     vk_aspect_flags_, 0U, specification_.mip_count, 0U, specification_.layers);

    commad_buffer->end();
    commad_buffer->submit();

    // Clean up the staging buffer
    vkDestroyBuffer(vk_logical_device, std::get<0>(vk_stagin_buffer_and_memory), vulkan_instance.allocation_callbaks);
    vkFreeMemory(vk_logical_device, std::get<1>(vk_stagin_buffer_and_memory), vulkan_instance.allocation_callbaks);

    // Create a struct containing information needed to create an image view
    VkImageViewCreateInfo vk_image_view_create_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};

    vk_image_view_create_info.pNext = VK_NULL_HANDLE;
    vk_image_view_create_info.flags = 0U;
    vk_image_view_create_info.image = vk_image_;
    vk_image_view_create_info.viewType =
        (specification_.layers > 1)    ? (specification_.is_cube_map) ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_2D_ARRAY
        : (specification_.is_cube_map) ? VK_IMAGE_VIEW_TYPE_CUBE
                                       : VK_IMAGE_VIEW_TYPE_2D; // If the image has layers, create a 2D ImageArray, else, create a 2D image view
    vk_image_view_create_info.format                          = vk_image_format_; // The format of the image view to be created
    vk_image_view_create_info.components                      = utils::getComponetsSwizzle(specification_.format, vk_image_usage_flags);
    vk_image_view_create_info.subresourceRange.aspectMask     = vk_aspect_flags_;
    vk_image_view_create_info.subresourceRange.baseMipLevel   = 0U;
    vk_image_view_create_info.subresourceRange.levelCount     = specification_.mip_count; // Number of levels in the mip chain
    vk_image_view_create_info.subresourceRange.baseArrayLayer = 0U;                       // Starting point of the layer to be accessed
    vk_image_view_create_info.subresourceRange.layerCount     = specification_.layers;    // Number of layers to be accessed

    VK_CHECK_RESULT(vkCreateImageView(vk_logical_device, &vk_image_view_create_info, vulkan_instance.allocation_callbaks, &vk_image_view_),
                    "Failed to create image view!");
    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_IMAGE_VIEW>(vulkan_instance.instance, "Vulkan main image veiw", vk_logical_device, vk_image_view_);

    // Create view per mip level
    for (std::uint32_t i{0}; i < specification_.mip_count; ++i)
    {
        // Create a struct containing information needed to create an image view
        VkImageViewCreateInfo vk_image_view_create_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};

        vk_image_view_create_info.pNext      = VK_NULL_HANDLE;
        vk_image_view_create_info.flags      = 0U;
        vk_image_view_create_info.image      = vk_image_;
        vk_image_view_create_info.viewType   = (specification_.layers > 1U) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
        vk_image_view_create_info.format     = vk_image_format_; // The format of the image view to be created
        vk_image_view_create_info.components = utils::getComponetsSwizzle(specification_.format, vk_image_usage_flags);

        vk_image_view_create_info.subresourceRange.aspectMask     = vk_aspect_flags_;
        vk_image_view_create_info.subresourceRange.baseMipLevel   = i;
        vk_image_view_create_info.subresourceRange.levelCount     = 1U;
        vk_image_view_create_info.subresourceRange.baseArrayLayer = 0U;
        vk_image_view_create_info.subresourceRange.layerCount     = specification_.layers;

        VK_CHECK_RESULT(vkCreateImageView(vk_logical_device, &vk_image_view_create_info, vulkan_instance.allocation_callbaks,
                                          &vk_image_views_per_mip_.emplace_back()),
                        "Failed to create image view!");
        vk_utils::setDebugObjectName<VK_OBJECT_TYPE_IMAGE_VIEW>(vulkan_instance.instance, "Vulkan image veiw per mip", vk_logical_device,
                                                                vk_image_views_per_mip_.back());
    }
    // Create view per layer
    for (std::uint32_t i{0}; i < specification_.layers; ++i)
    {
        // Create a struct containing information needed to create an image view
        VkImageViewCreateInfo vk_image_view_create_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};

        vk_image_view_create_info.pNext      = VK_NULL_HANDLE;
        vk_image_view_create_info.flags      = 0U;
        vk_image_view_create_info.image      = vk_image_;
        vk_image_view_create_info.viewType   = VK_IMAGE_VIEW_TYPE_2D;
        vk_image_view_create_info.format     = vk_image_format_; // The format of the image view to be created
        vk_image_view_create_info.components = utils::getComponetsSwizzle(specification_.format, vk_image_usage_flags);

        vk_image_view_create_info.subresourceRange.aspectMask     = vk_aspect_flags_;
        vk_image_view_create_info.subresourceRange.baseMipLevel   = 0U;
        vk_image_view_create_info.subresourceRange.levelCount     = 1U;
        vk_image_view_create_info.subresourceRange.baseArrayLayer = i;
        vk_image_view_create_info.subresourceRange.layerCount     = 1U;

        VK_CHECK_RESULT(vkCreateImageView(vk_logical_device, &vk_image_view_create_info, vulkan_instance.allocation_callbaks,
                                          &vk_image_views_per_layer_.emplace_back()),
                        "Failed to create image view!");
        vk_utils::setDebugObjectName<VK_OBJECT_TYPE_IMAGE_VIEW>(vulkan_instance.instance, "Vulkan image veiw per layer", vk_logical_device,
                                                                vk_image_views_per_layer_.back());
    }
}

void vshade::render::VulkanImage2D::invalidate(Image::Specification const& specification)
{
    VulkanInstance&        vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const         vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();
    VkPhysicalDevice const vk_physical_device =
        RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getVkDevice();

    specification_ = specification;
    // Clamp max mips count
    std::uint32_t const max_mips = std::uint32_t(std::floor(std::log2(std::max(specification_.width, specification_.height))) + 1U);
    specification_.mip_count     = (specification_.mip_count <= max_mips) ? specification_.mip_count : max_mips;

    vk_image_layout_  = VK_IMAGE_LAYOUT_UNDEFINED;
    is_depth_         = vk_utils::isDepthFormat(specification_.format);
    is_depth_stencil_ = vk_utils::isDepthStencilFormat(specification_.format);
    vk_image_format_  = vk_utils::toVulkanImageFormat(specification_.format);

    // vk_aspect_flags_ = (is_depth_) ? VK_IMAGE_ASPECT_DEPTH_BIT : (is_depth_stencil_) ? VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    vk_aspect_flags_ = (is_depth_ || is_depth_stencil_) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageUsageFlags vk_image_usage_flags{getVkImageUsageFlags()};

    VkImageCreateInfo vk_image_create_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    vk_image_create_info.pNext       = VK_NULL_HANDLE;
    vk_image_create_info.flags       = (specification_.is_cube_map) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : static_cast<VkImageCreateFlags>(0x0);
    vk_image_create_info.imageType   = VK_IMAGE_TYPE_2D;
    vk_image_create_info.format      = vk_image_format_;
    vk_image_create_info.extent      = {specification_.width, specification_.height, 1U};
    vk_image_create_info.mipLevels   = specification_.mip_count;
    vk_image_create_info.arrayLayers = specification_.layers;
    vk_image_create_info.samples     = VK_SAMPLE_COUNT_1_BIT;
    vk_image_create_info.tiling      = VK_IMAGE_TILING_OPTIMAL;
    vk_image_create_info.usage       = vk_image_usage_flags;
    vk_image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO: Take a look !!
    vk_image_create_info.queueFamilyIndexCount = 0;
    vk_image_create_info.pQueueFamilyIndices   = VK_NULL_HANDLE;
    vk_image_create_info.initialLayout         = vk_image_layout_;

    // Create image
    VK_CHECK_RESULT(vkCreateImage(vk_logical_device, &vk_image_create_info, vulkan_instance.allocation_callbaks, &vk_image_),
                    "Failed to create image!");
    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_IMAGE>(vulkan_instance.instance, "Image", vk_logical_device, vk_image_);

    // Create and bind image memory
    vk_image_memory_ = vk_utils::createImageMemory<VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT>(
        vk_logical_device, vk_physical_device, vulkan_instance.instance, vulkan_instance.allocation_callbaks, vk_image_);

    std::shared_ptr<RenderCommandBuffer> layout_trasition_commad_buffer{
        RenderCommandBuffer::create(RenderCommandBuffer::Type::_PRIMARY_, RenderCommandBuffer::Family::_TRANSFER_)};

    layout_trasition_commad_buffer->begin();

    if (specification_.usage == Image::Usage::_TEXTURE_)
    {
        // If it is texture we need to set layout to shader read only
        layoutTransition(layout_trasition_commad_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer().vk_command_buffer,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 0U, specification_.mip_count,
                         0U, specification_.layers);
    }
    else if (specification_.usage == Image::Usage::_ATTACHMENT_)
    {
        layoutTransition(layout_trasition_commad_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer().vk_command_buffer,
                         (is_depth_)           ? VK_IMAGE_LAYOUT_GENERAL
                         : (is_depth_stencil_) ? VK_IMAGE_LAYOUT_GENERAL
                                               : VK_IMAGE_LAYOUT_GENERAL,
                         VK_ACCESS_NONE, VK_ACCESS_NONE, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         (is_depth_)           ? VK_IMAGE_ASPECT_DEPTH_BIT
                         : (is_depth_stencil_) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
                                               : VK_IMAGE_ASPECT_COLOR_BIT,
                         0U, specification_.mip_count, 0U, specification_.layers);
    }
    else if (specification_.usage == Image::Usage::_STORAGE_) // TODO: Finish for storage
    {
        layoutTransition(layout_trasition_commad_buffer->as<VulkanRenderCommandBuffer>().getVulkanCommandBuffer().vk_command_buffer,
                         VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,

                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,

                         (is_depth_)           ? VK_IMAGE_ASPECT_DEPTH_BIT
                         : (is_depth_stencil_) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
                                               : VK_IMAGE_ASPECT_COLOR_BIT,
                         0U, specification_.mip_count, 0U, specification_.layers);
    }
    else
    {
        VSHADE_CORE_ERROR("Invalid Image Usage parametr = 'None'!");
    }

    layout_trasition_commad_buffer->end();
    layout_trasition_commad_buffer->submit();

    // Create a struct containing information needed to create an image view
    VkImageViewCreateInfo vk_image_view_create_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};

    vk_image_view_create_info.pNext = VK_NULL_HANDLE;
    vk_image_view_create_info.flags = 0U;
    vk_image_view_create_info.image = vk_image_;
    vk_image_view_create_info.viewType =
        (specification_.layers > 1)    ? (specification_.is_cube_map) ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_2D_ARRAY
        : (specification_.is_cube_map) ? VK_IMAGE_VIEW_TYPE_CUBE
                                       : VK_IMAGE_VIEW_TYPE_2D; // If the image has layers, create a 2D ImageArray, else, create a 2D image view
    vk_image_view_create_info.format                          = vk_image_format_; // The format of the image view to be created
    vk_image_view_create_info.components                      = utils::getComponetsSwizzle(specification_.format, vk_image_usage_flags);
    vk_image_view_create_info.subresourceRange.aspectMask     = vk_aspect_flags_;
    vk_image_view_create_info.subresourceRange.baseMipLevel   = 0U;
    vk_image_view_create_info.subresourceRange.levelCount     = specification_.mip_count; // Number of levels in the mip chain
    vk_image_view_create_info.subresourceRange.baseArrayLayer = 0U;                       // Starting point of the layer to be accessed
    vk_image_view_create_info.subresourceRange.layerCount     = specification_.layers;    // Number of layers to be accessed

    VK_CHECK_RESULT(vkCreateImageView(vk_logical_device, &vk_image_view_create_info, vulkan_instance.allocation_callbaks, &vk_image_view_),
                    "Failed to create image view!");
    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_IMAGE_VIEW>(vulkan_instance.instance, "Vulkan main image veiw", vk_logical_device, vk_image_view_);

    // Create view per mip level;
    // for (std::uint32_t i = 0; i < specification_.MipLevels; i++)
    // {
    // 	// Create a struct containing information needed to create an image view
    // 	VkImageViewCreateInfo imageViewCreateInfo
    // 	{
    // 	   .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // The current structure type
    // 	   .pNext = VK_NULL_HANDLE, // A pointer to the next structure if there's any other info to be filled
    // 	   .flags = 0, // Optional flags
    // 	   .image = m_VkImage, // The VkImage to create an image view for
    // 	   .viewType = (specification_.Layers > 1) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, // If the image has layers, create a 2D
    // ImageArray, else, create a 2D image view 	   .format = m_ImageFormat, // The format of the image view to be created 	   .components =
    // utils::GetComponetsSwizzle(m_Specification.Format, usageFlags), 	   .subresourceRange =  // The range of the subresource to be accessed
    // 		  {
    // 			.aspectMask = m_AspectFlags, // The aspect of the subresource to access, which could be COLOR, or DEPTH
    // 			.baseMipLevel = i,  // Starting point of the level of the mip to access
    // 			.levelCount = 1, // Number of levels in the mip chain
    // 			.baseArrayLayer = 0, // Starting point of the layer to be accessed
    // 			.layerCount = m_Specification.Layers // Number of layers to be accessed
    // 		  }
    // 	};

    // 	// Create the image view based on the info passed
    // 	VK_CHECK_RESULT(vkCreateImageView(m_VkDevice, &imageViewCreateInfo, m_VkInstance.AllocationCallbaks, &m_VkImageViewsMips.emplace_back()),
    // "Failed to create image view!");
    // 	// Set a debug object name for the VKUtils object instance being used
    // 	// This is done to help debugging if the program fails
    // 	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Iamge view", (std::uint32_t)&m_VkImageViewsMips.back()),
    // m_VkDevice, VK_OBJECT_TYPE_IMAGE_VIEW, m_VkImageViewsMips.back());
    // }
    // // Create view per layer;
    // for (std::uint32_t i = 0; i < m_Specification.Layers; i++)
    // {
    // 	// Create a struct containing information needed to create an image view
    // 	VkImageViewCreateInfo imageViewCreateInfo
    // 	{
    // 	   .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // The current structure type
    // 	   .pNext = VK_NULL_HANDLE, // A pointer to the next structure if there's any other info to be filled
    // 	   .flags = 0, // Optional flags
    // 	   .image = m_VkImage, // The VkImage to create an image view for
    // 	   .viewType = VK_IMAGE_VIEW_TYPE_2D,
    // 	   .format = m_ImageFormat, // The format of the image view to be created
    // 	  .components = utils::GetComponetsSwizzle(m_Specification.Format, usageFlags),
    // 	   .subresourceRange =  // The range of the subresource to be accessed
    // 		  {
    // 			.aspectMask = m_AspectFlags,	// The aspect of the subresource to access, which could be COLOR, or DEPTH
    // 			.baseMipLevel = 0,				// Starting point of the level of the mip to access
    // 			.levelCount = 1,				// Number of levels in the mip chain
    // 			.baseArrayLayer = i,			// Starting point of the layer to be accessed
    // 			.layerCount = 1					// Number of layers to be accessed
    // 		  }
    // 	};

    // 	// Create the image view based on the info passed
    // 	VK_CHECK_RESULT(vkCreateImageView(m_VkDevice, &imageViewCreateInfo, m_VkInstance.AllocationCallbaks, &m_VkImageViewsLayers.emplace_back()),
    // "Failed to create image view!");
    // 	// Set a debug object name for the VKUtils object instance being used
    // 	// This is done to help debugging if the program fails
    // 	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Iamge view", (std::uint32_t)&m_VkImageViewsLayers.back()),
    // m_VkDevice, VK_OBJECT_TYPE_IMAGE_VIEW, m_VkImageViewsLayers.back());
    // }
}

void vshade::render::VulkanImage2D::invalidate(Image::Specification const& specification, void const* source)
{
    // For swapchain creating
    VulkanInstance& vulkan_instance   = RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance();
    VkDevice const  vk_logical_device = RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice();

    specification_ = specification;
    // Clamp mips
    std::uint32_t const max_mips = std::uint32_t(std::floor(std::log2(std::max(specification_.width, specification_.height))) + 1U);
    specification_.mip_count     = (specification_.mip_count <= max_mips) ? specification_.mip_count : max_mips;

    vk_image_layout_  = VK_IMAGE_LAYOUT_UNDEFINED;
    is_depth_         = vk_utils::isDepthFormat(specification_.format);
    is_depth_stencil_ = vk_utils::isDepthStencilFormat(specification_.format);
    vk_image_format_  = vk_utils::toVulkanImageFormat(specification_.format);

    // Determine the image aspects to be used, whether it is for color or depth
    vk_aspect_flags_ = (is_depth_) ? VK_IMAGE_ASPECT_DEPTH_BIT : (is_depth_stencil_) ? VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageUsageFlags vk_image_usage_flags{getVkImageUsageFlags()};

    // In case where image was created in swapchain
    is_remoute_destorying_ = true;
    vk_image_              = reinterpret_cast<VkImage>(const_cast<void*>(source));

    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_IMAGE>(vulkan_instance.instance, "Swap chain image", vk_logical_device, vk_image_);

    // Create a struct containing information needed to create an image view
    VkImageViewCreateInfo vk_image_view_create_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};

    vk_image_view_create_info.pNext    = VK_NULL_HANDLE;
    vk_image_view_create_info.flags    = 0U;
    vk_image_view_create_info.image    = vk_image_;
    vk_image_view_create_info.viewType = (specification_.layers > 1U)
                                             ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
                                             : VK_IMAGE_VIEW_TYPE_2D; // If the image has layers, create a 2D ImageArray, else, create a 2D image view
    vk_image_view_create_info.format   = vk_image_format_;
    // vk_image_view_create_info.components                      = utils::getComponetsSwizzle(specification_.format, vk_image_usage_flags);

    // TODO: temporary
    vk_image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    vk_image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    vk_image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    vk_image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    vk_image_view_create_info.subresourceRange.aspectMask     = vk_aspect_flags_;
    vk_image_view_create_info.subresourceRange.baseMipLevel   = 0U; // Starting point of the level of the mip to access
    vk_image_view_create_info.subresourceRange.levelCount     = specification_.mip_count;
    vk_image_view_create_info.subresourceRange.baseArrayLayer = 0U, // Starting point of the layer to be accessed
        vk_image_view_create_info.subresourceRange.layerCount = specification_.layers;

    VK_CHECK_RESULT(vkCreateImageView(vk_logical_device, &vk_image_view_create_info, vulkan_instance.allocation_callbaks, &vk_image_view_),
                    "Failed to create image view!");
    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_IMAGE_VIEW>(vulkan_instance.instance, "Swap chain image view", vk_logical_device, vk_image_view_);

    // Create view per mip level;
    for (std::uint32_t i{0U}; i < specification_.mip_count; ++i)
    {
        VkImageViewCreateInfo vk_image_view_create_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        vk_image_view_create_info.pNext = VK_NULL_HANDLE;
        vk_image_view_create_info.flags = 0U;
        vk_image_view_create_info.image = vk_image_;
        vk_image_view_create_info.viewType =
            (specification_.layers > 1U) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D; // If the image has layers, create a 2D
        // ImageArray, else, create a 2D image view
        vk_image_view_create_info.format                          = vk_image_format_;
        vk_image_view_create_info.components                      = utils::getComponetsSwizzle(specification_.format, vk_image_usage_flags);
        vk_image_view_create_info.subresourceRange.aspectMask     = vk_aspect_flags_;
        vk_image_view_create_info.subresourceRange.baseMipLevel   = i;
        vk_image_view_create_info.subresourceRange.levelCount     = 1U;
        vk_image_view_create_info.subresourceRange.baseArrayLayer = 0U;
        vk_image_view_create_info.subresourceRange.layerCount     = specification_.layers;

        VK_CHECK_RESULT(vkCreateImageView(vk_logical_device, &vk_image_view_create_info, vulkan_instance.allocation_callbaks,
                                          &vk_image_views_per_mip_.emplace_back()),
                        "Failed to create image view!");

        vk_utils::setDebugObjectName<VK_OBJECT_TYPE_IMAGE_VIEW>(vulkan_instance.instance, "Swap chain image view per mip", vk_logical_device,
                                                                vk_image_views_per_mip_.back());
    }
    // Create view per layer;
    for (std::uint32_t i{0}; i < specification_.layers; ++i)
    {
        // Create a struct containing information needed to create an image view
        VkImageViewCreateInfo vk_image_view_create_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        vk_image_view_create_info.pNext                           = VK_NULL_HANDLE;
        vk_image_view_create_info.flags                           = 0U;
        vk_image_view_create_info.image                           = vk_image_;
        vk_image_view_create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        vk_image_view_create_info.format                          = vk_image_format_;
        vk_image_view_create_info.components                      = utils::getComponetsSwizzle(specification_.format, vk_image_usage_flags);
        vk_image_view_create_info.subresourceRange.aspectMask     = vk_aspect_flags_;
        vk_image_view_create_info.subresourceRange.baseMipLevel   = 0U;
        vk_image_view_create_info.subresourceRange.levelCount     = 1U;
        vk_image_view_create_info.subresourceRange.baseArrayLayer = i;
        vk_image_view_create_info.subresourceRange.layerCount     = 1U;

        VK_CHECK_RESULT(vkCreateImageView(vk_logical_device, &vk_image_view_create_info, vulkan_instance.allocation_callbaks,
                                          &vk_image_views_per_layer_.emplace_back()),
                        "Failed to create image view!");

        vk_utils::setDebugObjectName<VK_OBJECT_TYPE_IMAGE_VIEW>(vulkan_instance.instance, "Swap chain image view per layer", vk_logical_device,
                                                                vk_image_views_per_layer_.back());
    }
}

void vshade::render::VulkanImage2D::layoutTransition(VkCommandBuffer vk_command_buffer, VkImageLayout vk_new_layout, VkAccessFlags vk_src_access_mask,
                                                     VkAccessFlags vk_dst_access_mask, VkPipelineStageFlags vk_src_stage,
                                                     VkPipelineStageFlags vk_dst_stage, VkImageAspectFlags vk_aspect_mask,
                                                     std::uint32_t mip_base_level, std::uint32_t mip_levels, std::uint32_t layer_base_level,
                                                     std::uint32_t layer_count)
{
    // Mby here shoudl be some static global transition command buffer or something
    VkImageMemoryBarrier vk_image_memory_barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    vk_image_memory_barrier.oldLayout                       = vk_image_layout_;
    vk_image_memory_barrier.newLayout                       = vk_new_layout;
    vk_image_memory_barrier.srcAccessMask                   = vk_src_access_mask;
    vk_image_memory_barrier.dstAccessMask                   = vk_dst_access_mask;
    vk_image_memory_barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    vk_image_memory_barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    vk_image_memory_barrier.image                           = vk_image_;
    vk_image_memory_barrier.subresourceRange.aspectMask     = vk_aspect_mask;
    vk_image_memory_barrier.subresourceRange.baseMipLevel   = mip_base_level;
    vk_image_memory_barrier.subresourceRange.levelCount     = mip_levels;
    vk_image_memory_barrier.subresourceRange.baseArrayLayer = layer_base_level;
    vk_image_memory_barrier.subresourceRange.layerCount     = layer_count;

    vkCmdPipelineBarrier(vk_command_buffer, vk_src_stage, vk_dst_stage, 0U, 0U, VK_NULL_HANDLE, 0U, VK_NULL_HANDLE, 1U, &vk_image_memory_barrier);
    vk_image_layout_ = vk_new_layout;
}

VkImageUsageFlags vshade::render::VulkanImage2D::getVkImageUsageFlags()
{
    VkImageUsageFlags vk_image_usage_flags{VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT};

    if (specification_.usage == Image::Usage::_ATTACHMENT_)
    {
        if (is_depth_ || is_depth_stencil_)
        {
            vk_image_usage_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | ((!is_depth_stencil_) ? VK_IMAGE_USAGE_STORAGE_BIT : 0U);
        }
        else
        {
            vk_image_usage_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        }
    }
    else if (specification_.usage == Image::Usage::_TEXTURE_)
    {
        vk_image_usage_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    else if (specification_.usage == Image::Usage::_STORAGE_)
    {
        vk_image_usage_flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getPhysicalDevice()->getVkDevice(),
                                        VK_FORMAT_B8G8R8A8_UNORM, &props);

    if (!(props.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
    {
        VSHADE_CORE_ERROR("UNSUPORTED")
    }

    return vk_image_usage_flags;
}

std::tuple<VkBuffer, VkDeviceMemory> vshade::render::VulkanImage2D::createVkBuffer(VulkanInstance vulkan_instance, VkDevice vk_logical_device,
                                                                                   VkBufferCreateInfo const*    pCreateInfo,
                                                                                   VkMemoryPropertyFlags const& properties,
                                                                                   VkPhysicalDevice             vk_physical_device)
{
    VkBuffer vk_buffer{VK_NULL_HANDLE};
    VK_CHECK_RESULT(vkCreateBuffer(vk_logical_device, pCreateInfo, vulkan_instance.allocation_callbaks, &vk_buffer),
                    "Failed to create image buffer!");

    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_BUFFER>(vulkan_instance.instance, "Vulkan image buffer", vk_logical_device, vk_buffer);

    // retrieves memory requirements for the buffer
    VkMemoryRequirements vk_memory_requirements;
    vkGetBufferMemoryRequirements(vk_logical_device, vk_buffer, &vk_memory_requirements);

    // allocates memory for the buffer
    VkMemoryAllocateInfo vk_memory_allocate_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    vk_memory_allocate_info.pNext           = VK_NULL_HANDLE;
    vk_memory_allocate_info.allocationSize  = vk_memory_requirements.size;
    vk_memory_allocate_info.memoryTypeIndex = vk_utils::findMemoryType(vk_physical_device, vk_memory_requirements.memoryTypeBits, properties);

    VkDeviceMemory vk_buffer_memory{VK_NULL_HANDLE};
    VK_CHECK_RESULT(vkAllocateMemory(vk_logical_device, &vk_memory_allocate_info, vulkan_instance.allocation_callbaks, &vk_buffer_memory),
                    "Failed to allocate memory!");

    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_DEVICE_MEMORY>(vulkan_instance.instance, "Vulkan image buffer memory", vk_logical_device,
                                                               vk_buffer_memory);

    // binds the buffer with the allocated memory
    vkBindBufferMemory(vk_logical_device, vk_buffer, vk_buffer_memory, 0U);

    return {vk_buffer, vk_buffer_memory};
}