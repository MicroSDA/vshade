#ifndef ENGINE_PLATFORM_RENDER_VULKAN_DESRIPTORS_VULKAN_DESCRIPTOR_SET_POOL_H
#define ENGINE_PLATFORM_RENDER_VULKAN_DESRIPTORS_VULKAN_DESCRIPTOR_SET_POOL_H

#include <engine/core/utility/hash.h>
#include <engine/platforms/render/vulkan/descriptors/vulkan_descriptor_set_layout.h>
#include <engine/platforms/render/vulkan/vulkan_render_context.h>
#include <map>
#include <vector>

namespace vshade
{
namespace render
{
namespace descriptor
{
class VulkanDescriptorSetPool final
{
    static constexpr std::uint32_t _MAX_SETS_PER_POOL_{10U};

public:
    explicit VulkanDescriptorSetPool(VulkanInstance const& vulkan_instance, VkDevice const vk_logical_device, VulkanDescriptorSetLayout const& layout,
                                     std::uint32_t max_sets_per_pull = _MAX_SETS_PER_POOL_);
    ~VulkanDescriptorSetPool();

    VkDescriptorSet allocateVkDescriptorSet();

    VulkanDescriptorSetLayout const& getVulkanDescriptorSetLayout() const
    {
        return vulkan_descriptor_set_layout_;
    }
    std::vector<VkDescriptorPool> const& getVkDescriptorPools() const
    {
        return vk_descriptor_pools_;
    }

private:
    VulkanDescriptorSetLayout const&                             vulkan_descriptor_set_layout_;
    std::vector<VkDescriptorPoolSize>                            vk_descriptor_pool_sizes_;
    std::vector<VkDescriptorPool>                                vk_descriptor_pools_;
    std::vector<std::uint32_t>                                   vk_descriptor_pools_sets_count_;
    ankerl::unordered_dense::map<VkDescriptorSet, std::uint32_t> vk_descriptor_pool_mapping_;
    std::uint32_t                                                max_sets_per_pull_{0U}, pool_index_{0U};
    VulkanInstance const&                                        vulkan_instance_;
    VkDevice const                                               vk_logical_device_;

    std::uint32_t findAvaliblePool(std::uint32_t search_index);
};

} // namespace descriptor
} // namespace render
} // namespace vshade

namespace std
{
template <> struct hash<vshade::render::descriptor::VulkanDescriptorSetPool>
{
    std::size_t operator()(vshade::render::descriptor::VulkanDescriptorSetPool const& set) const
    {
        std::size_t result{0U};
        vshade::utils::hash_combine(result, set.getVulkanDescriptorSetLayout());
        return result;
    }
};
} // namespace std

#endif // ENGINE_PLATFORM_RENDER_VULKAN_DESRIPTORS_VULKAN_DESCRIPTOR_SET_POOL_H