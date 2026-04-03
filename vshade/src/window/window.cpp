#include "engine/core/window/window.h"
#include <engine/platforms/system/linux/vulkan_linux_window.h>

vshade::window::Window::Window(Properties const& properties) : properties_{properties}
{
}
void vshade::window::Window::create(Properties const& properties)
{
#if defined(_WIN32)
    //  Linux
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        CRTPSingleton<Window>::create<VulkanLinuxWindow>(properties);
        break;
    }
#elif defined(__linux__)
    //  Linux
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        CRTPSingleton<Window>::create<VulkanLinuxWindow>(properties);
        break;
    }
#endif
}

bool vshade::window::Window::onCloseEvent(std::shared_ptr<event::WindowClose> const event)
{
    return false;
}

bool vshade::window::Window::onResizeEvent(std::shared_ptr<event::WindowResize> const event)
{
    properties_.width  = event->getWidth();
    properties_.height = event->getHeight();

    // This should be sync operation, like requestResize
    return false;
}

std::uint32_t vshade::window::Window::getWidth()
{
    return properties_.width;
}

std::uint32_t vshade::window::Window::getHeight()
{
    return properties_.height;
}

void vshade::window::Window::destroy()
{
    CRTPSingleton<Window>::destroy();
}