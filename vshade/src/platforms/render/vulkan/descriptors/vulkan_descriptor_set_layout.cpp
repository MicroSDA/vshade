#include "engine/platforms/render/vulkan/descriptors/vulkan_descriptor_set_layout.h"

vshade::render::descriptor::VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanInstance const& vulkan_instance,
                                                                                 VkDevice const        vk_logical_device,
                                                                                 std::initializer_list<VkDescriptorSetLayoutBinding> const& bindings)
    : vk_descriptor_set_layout_bindings_{bindings}, vulkan_instance_{vulkan_instance}, vk_logical_device_{vk_logical_device}
{
    invalidate();
}

vshade::render::descriptor::VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanInstance const&                            vulkan_instance,
                                                                                 VkDevice const                                   vk_logical_device,
                                                                                 std::vector<VkDescriptorSetLayoutBinding> const& bindings)
    : vk_descriptor_set_layout_bindings_{bindings}, vulkan_instance_{vulkan_instance}, vk_logical_device_{vk_logical_device}
{
    invalidate();
}

vshade::render::descriptor::VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
    VulkanInstance& vulkan_instance{RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance()};
    VkDevice const  vk_logical_device{RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice()};

    if (vk_descriptor_set_layout_ != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(vk_logical_device, vk_descriptor_set_layout_, vulkan_instance.allocation_callbaks);
    }
}

vshade::render::descriptor::VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept
    : vk_descriptor_set_layout_create_info_{other.vk_descriptor_set_layout_create_info_}
    , vk_descriptor_set_layout_bindings_{std::move(other.vk_descriptor_set_layout_bindings_)}
    , vk_descriptor_set_layout_bindings_map_{std::move(other.vk_descriptor_set_layout_bindings_map_)}
    , vk_descriptor_set_layout_{other.vk_descriptor_set_layout_}
    , vulkan_instance_{other.vulkan_instance_}
    , vk_logical_device_{other.vk_logical_device_}
{
    other.vk_descriptor_set_layout_ = VK_NULL_HANDLE;
}

void vshade::render::descriptor::VulkanDescriptorSetLayout::invalidate()
{
    std::vector<VkDescriptorBindingFlags> vk_descriptor_binding_flags(
        vk_descriptor_set_layout_bindings_.size(), VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);

    VkDescriptorSetLayoutBindingFlagsCreateInfo vk_descriptor_set_layout_binding_flags_create_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO};

    vk_descriptor_set_layout_binding_flags_create_info.pNext         = VK_NULL_HANDLE;
    vk_descriptor_set_layout_binding_flags_create_info.pBindingFlags = vk_descriptor_binding_flags.data();
    vk_descriptor_set_layout_binding_flags_create_info.bindingCount  = static_cast<std::uint32_t>(vk_descriptor_binding_flags.size());

    vk_descriptor_set_layout_create_info_.pNext        = &vk_descriptor_set_layout_binding_flags_create_info;
    vk_descriptor_set_layout_create_info_.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    vk_descriptor_set_layout_create_info_.bindingCount = static_cast<std::uint32_t>(vk_descriptor_set_layout_bindings_.size());
    vk_descriptor_set_layout_create_info_.pBindings    = vk_descriptor_set_layout_bindings_.data();

    VulkanInstance& vulkan_instance{RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance()};
    VkDevice const  vk_logical_device{RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice()};

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(vk_logical_device, &vk_descriptor_set_layout_create_info_, vulkan_instance.allocation_callbaks,
                                                &vk_descriptor_set_layout_),
                    "Failed to create descriptor set layout !")
    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT>(vulkan_instance.instance, "Vulkan descriptor set layout", vk_logical_device,
                                                                       vk_descriptor_set_layout_);
    for (VkDescriptorSetLayoutBinding& vk_descriptor_set_layout_binding : vk_descriptor_set_layout_bindings_)
    {
        vk_descriptor_set_layout_bindings_map_.emplace(vk_descriptor_set_layout_binding.binding, &vk_descriptor_set_layout_binding);
    }

    // if (vk_descriptor_set_layout_bindings_.empty())
    // {
    //     VSHADE_CORE_DEBUG("VulkanDescriptorSetLayout has been created with VkDescriptorSetLayoutBinding count 0.");
    // }
}
