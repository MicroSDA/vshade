#ifndef ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_RENDER_CONTEXT_H
#define ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_RENDER_CONTEXT_H

#include <engine/core/logs/loger.h>
#include <engine/core/render/render_context.h>
#include <engine/platforms/render/vulkan/vulkan_device.h>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace vshade
{
namespace render
{
struct VulkanInstance
{
    VkInstance             instance{VK_NULL_HANDLE};
    VkAllocationCallbacks* allocation_callbaks{VK_NULL_HANDLE};
};

class VulkanRenderContext final : public RenderContext
{
public:
    explicit VulkanRenderContext() = default;
    virtual ~VulkanRenderContext();
    VulkanRenderContext(RenderContext const&)              = delete;
    VulkanRenderContext(RenderContext&&)                   = delete;
    VulkanRenderContext& operator=(RenderContext const&) & = delete;
    VulkanRenderContext& operator=(RenderContext&&) &      = delete;

    VulkanInstance& getVulkanInstance() // make as cosnt ?
    {
        return vulkan_instance_;
    }

    std::shared_ptr<VulkanLogicalDevice> getLogicalDevice()
    {
        return vulkan_logical_device_;
    }
    virtual void intitialize() override;
    virtual void shutDown() override;

private:
    VulkanInstance                       vulkan_instance_;
    std::shared_ptr<VulkanLogicalDevice> vulkan_logical_device_;

#ifdef _VSHADE_DEBUG_
    VkDebugUtilsMessengerEXT debug_utils_messenger_ext_{VK_NULL_HANDLE};
    void                     createDebugMessanger();
#endif
};
} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_RENDER_CONTEXT_H