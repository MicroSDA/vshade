#include "engine/platforms/render/vulkan/vulkan_device.h"
#include <engine/platforms/render/vulkan/vulkan_render_context.h>

namespace vshade
{
namespace render
{
namespace utils
{
static VkFormat findDepthFormat(VkPhysicalDevice& device)
{
    // Since all depth formats may be optional, we need to find a suitable depth format to use
    // Start with the highest precision packed format
    std::vector<VkFormat> depth_formats{VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT,
                                        VK_FORMAT_D16_UNORM};

    // TODO: Move to VulkanPhysicalDevice
    for (auto& format : depth_formats)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(device, format, &formatProps);
        // Format must support depth stencil attachment for optimal tiling
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            return format;
    }
    return VK_FORMAT_UNDEFINED;
}
} // namespace utils
} // namespace render
} // namespace vshade

vshade::render::VulkanPhysicalDevice::VulkanPhysicalDevice()
{
    std::uint32_t physical_device_count{0};

    VulkanRenderContext& vulkan_context = VulkanRenderContext::instance().as<VulkanRenderContext>();

    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(vulkan_context.getVulkanInstance().instance, &physical_device_count, VK_NULL_HANDLE),
                    "There are no devices which support Vulkan!");

    std::vector<VkPhysicalDevice> physical_devices{physical_device_count};
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(vulkan_context.getVulkanInstance().instance, &physical_device_count, physical_devices.data()),
                    "There are no devices which support Vulkan!");

    for (auto& device : physical_devices)
    {
        /* Getting all device propeties. */
        VkPhysicalDeviceMaintenance4Properties vk_physical_device_properties_4{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES};
        VkPhysicalDeviceProperties2            vk_physical_device_properties_2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};

        vk_physical_device_properties_2.pNext = &vk_physical_device_properties_4;
        vkGetPhysicalDeviceProperties2(device, &vk_physical_device_properties_2);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);

        VkPhysicalDeviceFeatures2 features2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        vkGetPhysicalDeviceFeatures2(device, &features2);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(device, &memory);

        if (deviceMeetsRequirements(vshade::System::instance().getConfiguration(), device, vk_physical_device_properties_2.properties.deviceType))
        {
            physical_device_                   = device;
            physical_device_features_          = features;
            physical_device_memory_properties_ = memory;
            vk_physical_device_properties_2_   = vk_physical_device_properties_2;
            vk_physical_device_properties_4_   = vk_physical_device_properties_4;

            if (physical_device_ != VK_NULL_HANDLE)
            {
                depth_format_ = utils::findDepthFormat(physical_device_);
            }

#ifdef _VSHADE_DEBUG_
            VSHADE_CORE_DEBUG("======================== Enabled device extension =========================");
            for (auto const& extension : required_extensions_)
            {
                VSHADE_CORE_DEBUG("Enabled extension: {0}", extension);
            }
            VSHADE_CORE_DEBUG("===========================================================================");
#endif // _VSHADE_DEBUG_

            break;
        }
    }
}

vshade::render::VulkanLogicalDevice::~VulkanLogicalDevice()
{
    VulkanInstance& vulkan_instance{VulkanRenderContext::instance().as<VulkanRenderContext>().getVulkanInstance()};

    physical_device_.reset();

    vkDestroyCommandPool(vk_device_, graphic_command_pool_, vulkan_instance.allocation_callbaks);
    vkDestroyCommandPool(vk_device_, transfer_command_pool_, vulkan_instance.allocation_callbaks);
    vkDestroyCommandPool(vk_device_, compute_command_pool, vulkan_instance.allocation_callbaks);
    vkDeviceWaitIdle(vk_device_);
    vkDestroyDevice(vk_device_, vulkan_instance.allocation_callbaks);
}

void vshade::render::VulkanPhysicalDevice::fetchQueueFamilyIndices(int const& flags)
{
    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if (flags & VK_QUEUE_COMPUTE_BIT)
    {
        for (uint32_t i{0U}; i < queue_family_properties_.size(); ++i)
        {
            VkQueueFamilyProperties const& queue_family_properties = queue_family_properties_[i];

            if ((queue_family_properties.queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queue_family_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
            {
                queue_family_indices_.compute = i;
                break;
            }
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if (flags & VK_QUEUE_TRANSFER_BIT)
    {
        for (std::uint32_t i{0U}; i < queue_family_properties_.size(); ++i)
        {
            VkQueueFamilyProperties const& queue_family_properties = queue_family_properties_[i];
            if ((queue_family_properties.queueFlags & VK_QUEUE_TRANSFER_BIT) && ((queue_family_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
                ((queue_family_properties.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
            {
                queue_family_indices_.transfer = i;
                break;
            }
        }
    }

    // For other queue types or if no separate compute queue is present, return the first one to
    // support the requested flags
    for (std::uint32_t i{0U}; i < queue_family_properties_.size(); ++i)
    {
        if ((flags & VK_QUEUE_TRANSFER_BIT) && queue_family_indices_.transfer == -1)
        {
            if (queue_family_properties_[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
                queue_family_indices_.transfer = i;
        }

        if ((flags & VK_QUEUE_COMPUTE_BIT) && queue_family_indices_.compute == -1)
        {
            if (queue_family_properties_[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
                queue_family_indices_.compute = i;
        }

        if (flags & VK_QUEUE_GRAPHICS_BIT)
        {
            if (queue_family_properties_[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                queue_family_indices_.graphics = i;
        }
    }
}

bool vshade::render::VulkanPhysicalDevice::deviceMeetsRequirements(System::Configuration const& configuration, VkPhysicalDevice& device,
                                                                   VkPhysicalDeviceType const& device_type)
{
    switch (configuration.gpu_type)
    {
    case System::GpuType::_INTEGRATED_:
        if (device_type != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            return false;
        }

        break;
    case System::GpuType::_DESCRATE_:
        if (device_type != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            return false;
        }
        break;

    default:
        break;
    }

    // Trying to find family queue count.
    std::uint32_t queue_family_count{0};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0U);
    if (queue_family_count == 0U)
    {
        VSHADE_CORE_ERROR("Physical device queue family count is 0!");
    }

    queue_family_properties_.resize(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_family_properties_.data());

    uint32_t extension_count{0};

    VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr), "Failed to get device extension properties!");

    if (extension_count > 0U)
    {
        std::vector<VkExtensionProperties> extensions{extension_count};
        if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, &extensions.front()) == VK_SUCCESS)
        {
            VSHADE_CORE_DEBUG("Selected physical device has {0} extensions.", extensions.size());
            for (auto const& extension : extensions)
            {
                supported_extensions_.emplace(extension.extensionName);
            }
        }
    }
    else
    {
        return false;
    }

    // As far as we are working with swapchain we need this extension to be enabled.
    required_extensions_.emplace_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    required_extensions_.emplace_back(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
    required_extensions_.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    required_extensions_.emplace_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    required_extensions_.emplace_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    required_extensions_.emplace_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
    required_extensions_.emplace_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
    required_extensions_.emplace_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

    bool is_not_found{false};
    for (auto const& extension : required_extensions_)
    {
        if (supported_extensions_.find(extension) == supported_extensions_.end())
        {
            VSHADE_CORE_WARNING("Graphic device doesn't support required exstension: {0}", extension);
        }
    }
    // Just to print them all and quit after.
    if (is_not_found)
    {
        return false;
    }

    static float const default_queue_priority{0.0f};
    // TODO: Get from requirements
    int const requested_queue_types{VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT};

    fetchQueueFamilyIndices(requested_queue_types);
    // Graphics queue
    if (requested_queue_types & VK_QUEUE_GRAPHICS_BIT)
    {
        VkDeviceQueueCreateInfo queue_info{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queue_info.queueFamilyIndex = queue_family_indices_.graphics;
        queue_info.queueCount       = 1U;
        queue_info.pQueuePriorities = &default_queue_priority;
        queue_create_infos_.push_back(queue_info);
    }
    // Compute queue
    if (requested_queue_types & VK_QUEUE_COMPUTE_BIT)
    {
        if (queue_family_indices_.compute != queue_family_indices_.graphics)
        {
            VkDeviceQueueCreateInfo queue_info{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
            queue_info.queueFamilyIndex = queue_family_indices_.compute;
            queue_info.queueCount       = 1U;
            queue_info.pQueuePriorities = &default_queue_priority;
            queue_create_infos_.push_back(queue_info);
        }
    }
    // Transfer queue
    if (requested_queue_types & VK_QUEUE_TRANSFER_BIT)
    {
        if ((queue_family_indices_.transfer != queue_family_indices_.graphics) && (queue_family_indices_.transfer != queue_family_indices_.compute))
        {
            // If compute family index differs, we need an additional queue create info for the
            // compute queue
            VkDeviceQueueCreateInfo queue_info{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
            queue_info.queueFamilyIndex = queue_family_indices_.transfer;
            queue_info.queueCount       = 1U;
            queue_info.pQueuePriorities = &default_queue_priority;
            queue_create_infos_.push_back(queue_info);
        }
    }

    VSHADE_CORE_DEBUG("Graphic device queue family indices : graphics '{}', compute '{}', transfer '{}'", queue_family_indices_.graphics,
                      queue_family_indices_.compute, queue_family_indices_.transfer);

    if (queue_family_indices_.graphics > -1 && queue_family_indices_.transfer > -1 && queue_family_indices_.compute > -1)
    {
        return true;
    }

    return false;
}

vshade::render::VulkanLogicalDevice::VulkanLogicalDevice(std::shared_ptr<VulkanPhysicalDevice> physical_device) : physical_device_{physical_device}
{
    if (!physical_device_->isDeviceReady())
    {
        VSHADE_CORE_ERROR("Failed to find physical device that meets requirements!")
    }

    VkDeviceCreateInfo vk_device_create_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    vk_device_create_info.flags                = 0;
    vk_device_create_info.queueCreateInfoCount = static_cast<std::uint32_t>(physical_device_->getDeviceQueueCreateInfos().size());
    vk_device_create_info.pQueueCreateInfos    = physical_device_->getDeviceQueueCreateInfos().data();
    vk_device_create_info.pEnabledFeatures     = &physical_device_->getPhysicalDeviceFeatures();

    std::uint32_t const required_extension_size{static_cast<std::uint32_t>(physical_device_->getRequiredExtensions().size())};

    if (required_extension_size > 0U)
    {
        vk_device_create_info.enabledExtensionCount   = required_extension_size;
        vk_device_create_info.ppEnabledExtensionNames = physical_device_->getRequiredExtensions().data();
        vk_device_create_info.pEnabledFeatures        = VK_NULL_HANDLE; // <--- Mandatory!

        // Vulkan 1.2
        vk_physical_device_vulkan_12_features_.sType                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        vk_physical_device_vulkan_12_features_.shaderOutputLayer         = VK_TRUE;
        vk_physical_device_vulkan_12_features_.shaderOutputViewportIndex = VK_TRUE;
        vk_physical_device_vulkan_12_features_.descriptorIndexing        = VK_TRUE;
        // Bind less descriptors
        vk_physical_device_vulkan_12_features_.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
        vk_physical_device_vulkan_12_features_.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
        vk_physical_device_vulkan_12_features_.descriptorBindingStorageImageUpdateAfterBind  = VK_TRUE;

        vk_physical_device_vulkan_12_features_.descriptorBindingPartiallyBound              = VK_TRUE;
        vk_physical_device_vulkan_12_features_.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        vk_physical_device_vulkan_12_features_.runtimeDescriptorArray                       = VK_TRUE;
        vk_physical_device_vulkan_12_features_.descriptorBindingVariableDescriptorCount     = VK_TRUE;
        vk_physical_device_vulkan_12_features_.shaderSampledImageArrayNonUniformIndexing    = VK_TRUE;

        // Vulkan 1.3
        vk_physical_device_vulkan_13_features_.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        vk_physical_device_vulkan_13_features_.dynamicRendering = VK_TRUE;
        vk_physical_device_vulkan_13_features_.maintenance4     = VK_TRUE;
        vk_physical_device_vulkan_13_features_.pNext            = &vk_physical_device_vulkan_12_features_;

        // Vulkan 2.0
        vk_physical_device_vulkan_2_features_.features.samplerAnisotropy = VK_TRUE;
        vk_physical_device_vulkan_2_features_.features.wideLines         = VK_TRUE;
        vk_physical_device_vulkan_2_features_.pNext                      = &vk_physical_device_vulkan_13_features_;
        vk_physical_device_vulkan_2_features_.features.geometryShader    = VK_TRUE;

        vk_device_create_info.pEnabledFeatures = VK_NULL_HANDLE; // <--- Mandatory!
        vk_device_create_info.pNext            = &vk_physical_device_vulkan_2_features_;
    }

    VK_CHECK_RESULT(vkCreateDevice(physical_device_->getVkDevice(), &vk_device_create_info, VK_NULL_HANDLE, &vk_device_),
                    "Failed to create vulkan logical device!");

    // Get a graphics queue from the device.
    vkGetDeviceQueue(vk_device_, physical_device_->getQueueFamilyIndices().graphics, 0, &vulkan_queues_.graphic);
    // Get a transfer queue from the device.
    vkGetDeviceQueue(vk_device_, physical_device_->getQueueFamilyIndices().transfer, 0, &vulkan_queues_.transfer);
    // Get a compute queue from the device.
    vkGetDeviceQueue(vk_device_, physical_device_->getQueueFamilyIndices().compute, 0, &vulkan_queues_.compute);

    // INFO: Present queue in Swapchain !

    VkCommandPoolCreateInfo vk_command_pool_create_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    vk_command_pool_create_info.pNext = VK_NULL_HANDLE;
    vk_command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vk_command_pool_create_info.queueFamilyIndex = physical_device_->getQueueFamilyIndices().graphics;
    VK_CHECK_RESULT(vkCreateCommandPool(vk_device_, &vk_command_pool_create_info, VK_NULL_HANDLE, &graphic_command_pool_),
                    "Failed to create graphic command pool!");

    vk_command_pool_create_info.queueFamilyIndex = physical_device_->getQueueFamilyIndices().transfer;
    VK_CHECK_RESULT(vkCreateCommandPool(vk_device_, &vk_command_pool_create_info, VK_NULL_HANDLE, &transfer_command_pool_),
                    "Failed to create transfer command pool!");

    vk_command_pool_create_info.queueFamilyIndex = physical_device_->getQueueFamilyIndices().compute;
    VK_CHECK_RESULT(vkCreateCommandPool(vk_device_, &vk_command_pool_create_info, VK_NULL_HANDLE, &compute_command_pool),
                    "Failed to create compute command pool!");
}

std::shared_ptr<vshade::render::VulkanPhysicalDevice> vshade::render::VulkanLogicalDevice::getPhysicalDevice()
{
    return physical_device_;
}
