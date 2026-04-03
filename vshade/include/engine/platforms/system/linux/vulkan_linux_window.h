#ifndef ENGINE_PLATFORMS_SYSTEM_VULKAN_LINUX_WINDOW_H
#define ENGINE_PLATFORMS_SYSTEM_VULKAN_LINUX_WINDOW_H

#include <engine/platforms/system/linux/linux_window.h>

namespace vshade
{
namespace window
{
class VulkanLinuxWindow final : public LinuxWindow
{
public:
    explicit VulkanLinuxWindow(Properties const& properties);
    virtual ~VulkanLinuxWindow();
    VulkanLinuxWindow(VulkanLinuxWindow const&)              = delete;
    VulkanLinuxWindow(VulkanLinuxWindow&&)                   = delete;
    VulkanLinuxWindow& operator=(VulkanLinuxWindow const&) & = delete;
    VulkanLinuxWindow& operator=(VulkanLinuxWindow&&) &      = delete;
};
} // namespace window
} // namespace vshade
#endif // ENGINE_PLATFORMS_SYSTEM_VULKAN_LINUX_WINDOW_H