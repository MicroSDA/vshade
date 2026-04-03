#include "engine/core/render/swap_chain.h"
#include <engine/platforms/render/vulkan/vulkan_swap_chain.h>

void vshade::render::SwapChain::create()
{
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        utility::CRTPSingleton<SwapChain>::create<VulkanSwapChain>();
        break;
    }
}

void vshade::render::SwapChain::destroy()
{
    utility::CRTPSingleton<SwapChain>::destroy();
}