#include "engine/platforms/render/vulkan/vulkan_render_context.h"
#include <glfw/include/GLFW/glfw3.h>

namespace vshade
{
namespace utils
{
static bool checkLayersSupport(std::vector<char const*>& layers)
{
    std::uint32_t layers_count{0U};
    vkEnumerateInstanceLayerProperties(&layers_count, VK_NULL_HANDLE);
    std::vector<VkLayerProperties> available_layers(layers_count);
    vkEnumerateInstanceLayerProperties(&layers_count, available_layers.data());

    for (std::size_t i{0U}; i < layers.size(); ++i)
    {
        bool found{false};

        for (auto const& aL : available_layers)
        {
            if (strcmp(layers[i], aL.layerName) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            VSHADE_CORE_WARNING("Requested '{}' is not supported!", layers[i]);
            layers.erase(layers.begin() + i);
        }
    }

    return layers.size() > 0U;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vulkanMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
                                                     VkDebugUtilsMessengerCallbackDataEXT const* data, void* user_data)
{
    switch (severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        VSHADE_CORE_TRACE("VK_RENDER_INFO: {0}", data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        VSHADE_CORE_INFO("VK_RENDER_INFO: {0}", data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        VSHADE_CORE_WARNING("VK_RENDER_INFO: {0}", data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        VSHADE_CORE_ERROR("VK_RENDER_INFO: {0}", data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
        break;
    default:
        break;
    }
    return VK_FALSE;
}
} // namespace utils
} // namespace vshade

vshade::render::VulkanRenderContext::~VulkanRenderContext()
{
}

void vshade::render::VulkanRenderContext::intitialize()
{
    // Check Vulkan Drivers version
    std::uint32_t instanceVersion{0U};
    VK_CHECK_RESULT(vkEnumerateInstanceVersion(&instanceVersion), "Failed to get Vulkan instance version : " + __LINE__);

    if (instanceVersion < VK_API_VERSION_1_3)
    {
        VSHADE_CORE_ERROR("Engine requires at least Vulkan version {}.{}.{}", VK_API_VERSION_MAJOR(VK_API_VERSION_1_3),
                          VK_API_VERSION_MINOR(VK_API_VERSION_1_3), VK_API_VERSION_PATCH(VK_API_VERSION_1_3));
    }

    /* Getting required extension */
    std::uint32_t requiredExtensionCount{0U};
    char const**  requiredExtensions;
    requiredExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);
    // Create set of extensions
    std::vector<char const*> instance_extensions;
    for (std::uint32_t i = 0; i < requiredExtensionCount; i++)
    {
        instance_extensions.emplace_back(requiredExtensions[i]);
    }

    std::vector<char const*> layers;
#ifdef _VSHADE_DEBUG_
    // Add extension for debug messanger.
    instance_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    instance_extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    // instance_extensions.emplace_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    // instance_extensions.emplace_back("VK_EXT_depth_range_unrestricted");

    // Enable validation layers for debug purposes.
    layers.emplace_back("VK_LAYER_KHRONOS_validation");

    // Print everething into console
    // layers.emplace_back("VK_LAYER_LUNARG_api_dump");

    utils::checkLayersSupport(layers);

    VSHADE_CORE_DEBUG("============================== Enabled layers =============================");
    for (auto const& layer : layers)
    {
        VSHADE_CORE_DEBUG("Enabled layer: {0}", layer);
    }
    VSHADE_CORE_DEBUG("===========================================================================");

#endif // _VSHADE_DEBUG_

    instance_extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance_extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(_WIN32)
    instance_extensions.emplace_back("VK_KHR_win32_surface");
#elif defined(__linux__)
    instance_extensions.emplace_back("VK_KHR_xcb_surface"); //  VK_KHR_XCB_SURFACE_EXTENSION_NAME
#endif

    VSHADE_CORE_DEBUG("======================= Enabled instance extensions =======================");
    for (auto const& extension : instance_extensions)
    {
        VSHADE_CORE_DEBUG("Enabled extension: {0}", extension);
    }
    VSHADE_CORE_DEBUG("===========================================================================");

    // Create application info.
    VkApplicationInfo vk_application_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    vk_application_info.apiVersion         = VK_API_VERSION_1_3; // At least vulkan 1.3
    vk_application_info.pEngineName        = "V-Shade";
    vk_application_info.engineVersion      = VK_MAKE_VERSION(0, 0, 1);
    vk_application_info.pApplicationName   = "V-Shade";                // TODO: Make conigurable.
    vk_application_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1); // TODO: Make conigurable.
    vk_application_info.pNext              = VK_NULL_HANDLE;
    // Create application instance info.
    VkInstanceCreateInfo vk_instance_create_info    = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    vk_instance_create_info.pApplicationInfo        = &vk_application_info;
    vk_instance_create_info.enabledExtensionCount   = static_cast<std::uint32_t>(instance_extensions.size());
    vk_instance_create_info.ppEnabledExtensionNames = instance_extensions.data(); // Enable extensions.
    vk_instance_create_info.enabledLayerCount       = static_cast<std::uint32_t>(layers.size());
    vk_instance_create_info.ppEnabledLayerNames     = layers.data(); // Enable layers.

#ifdef _VSHADE_DEBUG_
    static VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};
    static VkValidationFeaturesEXT      features{VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, nullptr, 1U, enables, 0U, nullptr};
    vk_instance_create_info.pNext = &features;
#endif // _VSHADE_DEBUG_
    // Creating Vulkan instance.
    VK_CHECK_RESULT(vkCreateInstance(&vk_instance_create_info, vulkan_instance_.allocation_callbaks, &vulkan_instance_.instance),
                    "Failed to create Vulkan instance !");

#ifdef _VSHADE_DEBUG_
    createDebugMessanger();
#endif // _VSHADE_DEBUG_

    // Create logical device based on the physical device.

    vulkan_logical_device_ = VulkanLogicalDevice::create<VulkanLogicalDevice>(VulkanPhysicalDevice::create<VulkanPhysicalDevice>());
    VkPhysicalDeviceProperties const&       device_roperties = vulkan_logical_device_->getPhysicalDevice()->getPhysicalDeviceProperties();
    VkPhysicalDeviceMemoryProperties const& device_memory_properties =
        vulkan_logical_device_->getPhysicalDevice()->getPhysicalDeviceMemoryProperties();

    VSHADE_CORE_INFO("===== Vulkan render initialized successfuly with following parameters =====");
    VSHADE_CORE_INFO("Selected GPU      : {}", device_roperties.deviceName);
    VSHADE_CORE_INFO("GPU Driver version: {0}.{1}.{2}", VK_VERSION_MAJOR(device_roperties.driverVersion),
                     VK_VERSION_MINOR(device_roperties.driverVersion), VK_VERSION_PATCH(device_roperties.driverVersion));

    VSHADE_CORE_INFO("Vulkan API version: {0}.{1}.{2}", VK_VERSION_MAJOR(device_roperties.apiVersion), VK_VERSION_MINOR(device_roperties.apiVersion),
                     VK_VERSION_PATCH(device_roperties.apiVersion));

    for (std::uint32_t j = 0; j < device_memory_properties.memoryHeapCount; j++)
    {
        float const memory_size = static_cast<float>(static_cast<float>(device_memory_properties.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
        if (device_memory_properties.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        {
            VSHADE_CORE_INFO("Local GPU memory  : {0} gib", memory_size);
        }
    }
    VSHADE_CORE_INFO("===========================================================================");
}

void vshade::render::VulkanRenderContext::shutDown()
{
#ifdef _VSHADE_DEBUG_
    if (debug_utils_messenger_ext_)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkan_instance_.instance, "vkDestroyDebugUtilsMessengerEXT");

        if (vkDestroyDebugUtilsMessengerEXT)
        {
            vkDestroyDebugUtilsMessengerEXT(vulkan_instance_.instance, debug_utils_messenger_ext_, vulkan_instance_.allocation_callbaks);
            debug_utils_messenger_ext_ = VK_NULL_HANDLE;
        }
    }
#endif // _VSHADE_DEBUG_

    deleteAllPendings(0U);
    vulkan_logical_device_.reset();
    vkDestroyInstance(vulkan_instance_.instance, vulkan_instance_.allocation_callbaks);
}

#ifdef _VSHADE_DEBUG_
void vshade::render::VulkanRenderContext::createDebugMessanger()
{
    std::uint32_t severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT vk_debug_utils_createInfo{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    vk_debug_utils_createInfo.messageSeverity = severity;
    vk_debug_utils_createInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    vk_debug_utils_createInfo.pfnUserCallback = utils::vulkanMessageCallback;

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkan_instance_.instance, "vkCreateDebugUtilsMessengerEXT");

    if (!vkCreateDebugUtilsMessengerEXT)

    {
        VSHADE_CORE_WARNING("Failed to create Vulkan debugger!")
    }
    else
    {
        if (vkCreateDebugUtilsMessengerEXT(vulkan_instance_.instance, &vk_debug_utils_createInfo, vulkan_instance_.allocation_callbaks,
                                           &debug_utils_messenger_ext_) != VK_SUCCESS)
        {
            VSHADE_CORE_ERROR("Failed to create Vulkan debugger!")
        }
        else
        {
            VSHADE_CORE_DEBUG("Vulkan debugger initialized successfully.");
        }
    }
}
#endif // SHADE_DEBUG