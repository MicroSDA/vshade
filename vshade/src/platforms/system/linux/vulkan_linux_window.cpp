#include "engine/platforms/system/linux/vulkan_linux_window.h"

vshade::window::VulkanLinuxWindow::VulkanLinuxWindow(Properties const& properties) : LinuxWindow(properties)
{
    // Set error callback
    glfwSetErrorCallback(gLFWErrorCallback);

    // Initialize GLFW
    if (!glfwInit())
    {
        VSHADE_CORE_ERROR("Couldn't initialize GLFW !");
    }

    // Chech Vulkan support
    if (glfwVulkanSupported() == GLFW_FALSE)
    {
        VSHADE_CORE_ERROR("GLFW must support Vulkan!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    if (properties_.no_title_bar)
    {
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    }

    GLFWvidmode const* video_mode{glfwGetVideoMode(glfwGetPrimaryMonitor())};

    properties_.width  = (properties_.width > static_cast<std::uint32_t>(video_mode->width) || properties_.width <= 0U)
                             ? static_cast<std::uint32_t>(video_mode->width)
                             : static_cast<std::uint32_t>(properties_.width);
    properties_.height = (properties_.height > static_cast<std::uint32_t>(video_mode->height) || properties_.height <= 0U)
                             ? static_cast<std::uint32_t>(video_mode->height)
                             : static_cast<std::uint32_t>(properties_.height);

    window_ptr_ = glfwCreateWindow((properties_.full_screen) ? video_mode->width : properties_.width,
                                   (properties_.full_screen) ? video_mode->height : properties_.height, properties_.title.c_str(),
                                   (properties_.full_screen) ? glfwGetPrimaryMonitor() : nullptr, nullptr);

    if (window_ptr_ == nullptr)
    {
        glfwTerminate();
        VSHADE_CORE_ERROR("Failed to create window !");
    }

    render::SwapChain::create();
    render::SwapChain::instance().createSurface(window_ptr_);
    render::SwapChain::instance().createFrame(properties_.width, properties_.height, System::instance().getConfiguration().frames_in_flight,
                                              properties_.v_sync);

    glfwSetWindowUserPointer(static_cast<GLFWwindow*>(window_ptr_), &System::instance().getUserData());
    initWindowEvents();
}
vshade::window::VulkanLinuxWindow::~VulkanLinuxWindow()
{
    render::SwapChain::destroy();
}