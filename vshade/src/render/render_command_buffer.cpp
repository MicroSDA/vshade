#include "engine/core/render/render_command_buffer.h"
#include <engine/platforms/render/vulkan/vulkan_render_command_buffer.h>

std::shared_ptr<vshade::render::RenderCommandBuffer> vshade::render::RenderCommandBuffer::create(Type const type, Family const family,
                                                                                                 std::uint32_t const count)
{
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        return utility::CRTPFactory<RenderCommandBuffer>::create<VulkanRenderCommandBuffer>(type, family, count);
    }
}

vshade::render::RenderCommandBuffer::RenderCommandBuffer(Type const type, Family const family, std::uint32_t const count)
    : type_{type}, family_{family}
{
    is_recordered_.resize(count, false);
    is_in_recorder_stage_.resize(count, false);
}