#ifndef ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_DEVICE_H
#define ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_DEVICE_H
#include <engine/config/vshade_api.h>
#include <engine/core/utility/singleton.h>

#include <engine/config/system.h>
#include <engine/core/utility/factory.h>
#include <engine/platforms/render/vulkan/vk_utils.h>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan.h>

// TODO: USE ANKREL MAP
namespace vshade
{
namespace render
{
struct QueueFamilyIndices
{
    std::int32_t present  = -1;
    std::int32_t graphics = -1;
    std::int32_t compute  = -1;
    std::int32_t transfer = -1;
};
struct VulkanQueues
{
    VkQueue graphic{VK_NULL_HANDLE};
    VkQueue transfer{VK_NULL_HANDLE};
    VkQueue compute{VK_NULL_HANDLE};
    VkQueue present{VK_NULL_HANDLE};
};

class VSHADE_API VulkanPhysicalDevice final : public utility::CRTPFactory<VulkanPhysicalDevice>
{
    friend class utility::CRTPFactory<VulkanPhysicalDevice>;

public:
    virtual ~VulkanPhysicalDevice()                                = default;
    VulkanPhysicalDevice(VulkanPhysicalDevice const&)              = delete;
    VulkanPhysicalDevice(VulkanPhysicalDevice&&)                   = delete;
    VulkanPhysicalDevice& operator=(VulkanPhysicalDevice const&) & = delete;
    VulkanPhysicalDevice& operator=(VulkanPhysicalDevice&&) &      = delete;

    VkPhysicalDeviceFeatures const& getPhysicalDeviceFeatures() const
    {
        return physical_device_features_;
    }
    VkPhysicalDeviceMemoryProperties const& getPhysicalDeviceMemoryProperties() const
    {
        return physical_device_memory_properties_;
    }
    VkPhysicalDeviceMaintenance4Properties const& getPhysicalDeviceProperties4() const
    {
        return vk_physical_device_properties_4_;
    }
    VkPhysicalDeviceProperties const& getPhysicalDeviceProperties() const
    {
        return vk_physical_device_properties_2_.properties;
    }
    VkPhysicalDevice getVkDevice()
    {
        return physical_device_;
    }
    VkFormat const& getDepthForamt() const
    {
        return depth_format_;
    }
    std::unordered_set<std::string> const& getSupportedExtensions() const
    {
        return supported_extensions_;
    }
    std::vector<char const*> const& getRequiredExtensions() const
    {
        return required_extensions_;
    }
    std::vector<VkDeviceQueueCreateInfo> const& getDeviceQueueCreateInfos() const
    {
        return queue_create_infos_;
    }
    QueueFamilyIndices const& getQueueFamilyIndices() const
    {
        return queue_family_indices_;
    }
    bool isDeviceReady() const
    {
        return (physical_device_ != VK_NULL_HANDLE);
    }

    void setQueueFamilyIndexPresent(std::uint32_t node_index)
    {
        queue_family_indices_.present = node_index;
    }

protected:
    explicit VulkanPhysicalDevice();

private:
    bool deviceMeetsRequirements(System::Configuration const& configuration, VkPhysicalDevice& device, VkPhysicalDeviceType const& deviceType);
    void fetchQueueFamilyIndices(int const& flags);
    std::vector<VkQueueFamilyProperties>   queue_family_properties_;
    std::vector<VkDeviceQueueCreateInfo>   queue_create_infos_;
    std::vector<char const*>               required_extensions_;
    std::unordered_set<std::string>        supported_extensions_;
    VkPhysicalDeviceFeatures               physical_device_features_;
    VkPhysicalDeviceMemoryProperties       physical_device_memory_properties_;
    VkPhysicalDeviceMaintenance4Properties vk_physical_device_properties_4_{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES};
    VkPhysicalDeviceProperties2            vk_physical_device_properties_2_{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    VkPhysicalDevice                       physical_device_{VK_NULL_HANDLE};
    VkFormat                               depth_format_{VK_FORMAT_UNDEFINED};
    QueueFamilyIndices                     queue_family_indices_;
};

class VulkanLogicalDevice final : public utility::CRTPFactory<VulkanLogicalDevice>
{
    friend class utility::CRTPFactory<VulkanLogicalDevice>;

public:
    virtual ~VulkanLogicalDevice();
    VulkanLogicalDevice(VulkanLogicalDevice const&)                               = delete;
    VulkanLogicalDevice(VulkanLogicalDevice&&)                                    = delete;
    VulkanLogicalDevice&                  operator=(VulkanLogicalDevice const&) & = delete;
    VulkanLogicalDevice&                  operator=(VulkanLogicalDevice&&) &      = delete;
    std::shared_ptr<VulkanPhysicalDevice> getPhysicalDevice();
    VkDevice const                        getVkDevice() const
    {
        return vk_device_;
    }
    VulkanQueues& getVulkanQueus()
    {
        return vulkan_queues_;
    }

protected:
    explicit VulkanLogicalDevice(std::shared_ptr<VulkanPhysicalDevice> physical_device);

private:
    std::shared_ptr<VulkanPhysicalDevice> physical_device_;
    QueueFamilyIndices                    getQueueFamilyIndices(int const& flags);
    VkDevice                              vk_device_{VK_NULL_HANDLE};
    VkPhysicalDeviceVulkan12Features      vk_physical_device_vulkan_12_features_{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    VkPhysicalDeviceVulkan13Features      vk_physical_device_vulkan_13_features_{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    VkPhysicalDeviceFeatures2             vk_physical_device_vulkan_2_features_{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

    VulkanQueues  vulkan_queues_;
    VkCommandPool graphic_command_pool_{VK_NULL_HANDLE}, transfer_command_pool_{VK_NULL_HANDLE}, compute_command_pool{VK_NULL_HANDLE};
};

} // namespace render

} // namespace vshade

#endif // ENGINE_CORE_PLATFORM_RENDER_VULKAN_VULKAN_DEVICE_H
