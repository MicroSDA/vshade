#include "engine/core/render/render_context.h"
#include <engine/platforms/render/vulkan/vulkan_render_context.h>

vshade::render::RenderContext::RenderContext() : delete_queue_{RenderCommandQueue::create<RenderCommandQueue>()}
{
}

vshade::render::RenderContext::~RenderContext()
{
  
}

void vshade::render::RenderContext::create(API api) 
{
    switch (api)
    {
    case API::_VULKAN_:
        utility::CRTPSingleton<RenderContext>::create<VulkanRenderContext>();
        break;

    case API::_OPEN_GL_:
        std::abort();
        break;
    default:
        break;
    }
}
void vshade::render::RenderContext::destroy()
{
    utility::CRTPSingleton<RenderContext>::instance().shutDown();
    utility::CRTPSingleton<RenderContext>::destroy();
}