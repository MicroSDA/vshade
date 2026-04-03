#include "engine/core/render/buffers/frame_buffer.h"
#include <engine/platforms/render/vulkan/vulkan_frame_buffer.h>

std::shared_ptr<vshade::render::FrameBuffer> vshade::render::FrameBuffer::create(Specification const& specification)
{
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        return utility::CRTPFactory<FrameBuffer>::create<VulkanFrameBuffer>(specification);
    }
}

std::shared_ptr<vshade::render::FrameBuffer> vshade::render::FrameBuffer::create(Specification const&                         specification,
                                                                                 std::vector<std::shared_ptr<Image2D>> const& images)
{
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        return utility::CRTPFactory<FrameBuffer>::create<VulkanFrameBuffer>(specification, images);
    }
}

vshade::render::FrameBuffer::FrameBuffer(Specification const& specification) : specification_{specification}
{
}
vshade::render::FrameBuffer::FrameBuffer(Specification const& specification, std::vector<std::shared_ptr<Image2D>> const& images)
    : specification_{specification}
{
}