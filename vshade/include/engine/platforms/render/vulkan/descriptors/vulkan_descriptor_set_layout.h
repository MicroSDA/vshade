#ifndef ENGINE_PLATFORM_RENDER_VULKAN_DESRIPTORS_VULKAN_DESCRIPTOR_SET_LAYOUT_H
#define ENGINE_PLATFORM_RENDER_VULKAN_DESRIPTORS_VULKAN_DESCRIPTOR_SET_LAYOUT_H

#include <ankerl/unordered_dense.h>
#include <engine/core/logs/loger.h>
#include <engine/core/utility/hash.h>
#include <engine/platforms/render/vulkan/vk_utils.h>
#include <engine/platforms/render/vulkan/vulkan_render_context.h>
#include <iostream>
#include <vector>

namespace vshade
{
namespace render
{
namespace descriptor
{
class VulkanDescriptorSetLayout final
{
public:
    VulkanDescriptorSetLayout(VulkanInstance const& vulkan_instance, VkDevice const vk_logical_device,
                              std::initializer_list<VkDescriptorSetLayoutBinding> const& bindings);
    VulkanDescriptorSetLayout(VulkanInstance const& vulkan_instance, VkDevice const vk_logical_device,
                              std::vector<VkDescriptorSetLayoutBinding> const& bindings);
    ~VulkanDescriptorSetLayout();

    VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept;
    VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&&) & noexcept      = delete;
    VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout const&) & noexcept = delete;
    VulkanDescriptorSetLayout(VulkanDescriptorSetLayout const&) noexcept              = delete;

    VkDescriptorSetLayout const& getVkDescriptorSetLayout() const
    {
        return vk_descriptor_set_layout_;
    }
    std::vector<VkDescriptorSetLayoutBinding> const& getDescriptorSetLayoutBindings() const
    {
        return vk_descriptor_set_layout_bindings_;
    }

    VkDescriptorSetLayoutBinding* const getDescriptorSetLayoutBinding(std::size_t const index) const
    {
        if (vk_descriptor_set_layout_bindings_map_.find(index) != vk_descriptor_set_layout_bindings_map_.end())
        {
            return vk_descriptor_set_layout_bindings_map_.at(index);
        }
        return nullptr;
    }

private:
    VkDescriptorSetLayoutCreateInfo           vk_descriptor_set_layout_create_info_{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    VkDescriptorSetLayout                     vk_descriptor_set_layout_{VK_NULL_HANDLE};
    std::vector<VkDescriptorSetLayoutBinding> vk_descriptor_set_layout_bindings_;
    ankerl::unordered_dense::map<std::size_t, VkDescriptorSetLayoutBinding*> vk_descriptor_set_layout_bindings_map_;

    VulkanInstance const& vulkan_instance_;
    VkDevice const        vk_logical_device_;

    void invalidate();
};
} // namespace descriptor
} // namespace render
} // namespace vshade
namespace std
{
template <> struct hash<vshade::render::descriptor::VulkanDescriptorSetLayout>
{
    std::size_t operator()(vshade::render::descriptor::VulkanDescriptorSetLayout const& layout) const
    {
        std::size_t result{0U};
        vshade::utils::hash_combine(result, layout.getVkDescriptorSetLayout());
        return result;
    }
};
} // namespace std

#endif // ENGINE_PLATFORM_RENDER_VULKAN_DESRIPTORS_VULKAN_DESCRIPTOR_SET_LAYOUT_H