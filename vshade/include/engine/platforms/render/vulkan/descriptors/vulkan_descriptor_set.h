#ifndef ENGINE_PLATFORM_RENDER_VULKAN_DESRIPTORS_VULKAN_DESCRIPTORS_SET_H
#define ENGINE_PLATFORM_RENDER_VULKAN_DESRIPTORS_VULKAN_DESCRIPTORS_SET_H

#include <ankerl/unordered_dense.h>
#include <engine/platforms/render/vulkan/descriptors/vulkan_descriptor_set_pool.h>
#include <engine/platforms/render/vulkan/vulkan_render_context.h>

namespace vshade
{
namespace render
{
namespace descriptor
{
template <typename T> using DescriptorsBindingsMap = ankerl::unordered_dense::map<std::size_t, ankerl::unordered_dense::map<std::size_t, T>>;

struct VulkanDescriptorBindings final
{
    DescriptorsBindingsMap<VkDescriptorBufferInfo> buffers;
    DescriptorsBindingsMap<VkDescriptorImageInfo>  images;
};

class VulkanDescriptorSet final
{
public:
    // enum class Set
    // {

    // };

    // https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorType.html
    enum Type : std::uint32_t
    {
        _SAMPLER_                = 0,
        _COMBINED_IMAGE_SAMPLER_ = 1,
        _SAMPLED_IMAGE_          = 2,
        _STORAGE_IMAGE_          = 3,
        _UNIFORM_TEXEL_BUFFER_   = 4,
        _STORAGE_TEXEL_BUFFER_   = 5,
        _UNIFORM_BUFFER_         = 6,
        _STORAGE_BUFFER_         = 7,
        _UNIFORM_BUFFER_DYNAMIC_ = 8,
        _STORAGE_BUFFER_DYNAMIC_ = 9,
        _INPUT_ATTACHMENT_       = 10
    };

    explicit VulkanDescriptorSet(VulkanInstance const& vulkan_instance, VkDevice const vk_logical_device, VulkanDescriptorSetLayout const& layout,
                                 std::shared_ptr<VulkanDescriptorSetPool> pool, VulkanDescriptorBindings const& descriptor_bindings);
    ~VulkanDescriptorSet() = default;

    void update(DescriptorsBindingsMap<VkDescriptorBufferInfo> const& buffer_infos, DescriptorsBindingsMap<VkDescriptorImageInfo> const& image_infos);
    VkDescriptorSet const& getVkDescriptorSet() const
    {
        return vk_descriptor_set_;
    }
    void setAsDepricated(bool is_depricated)
    {
        is_deprecated_ = is_depricated;
    }
    bool isDepricated() const
    {
        return is_deprecated_;
    }

private:
    VkDescriptorSet                          vk_descriptor_set_{VK_NULL_HANDLE};
    VulkanDescriptorSetLayout const&         vulkan_descriptor_set_layout_;
    std::shared_ptr<VulkanDescriptorSetPool> vulkane_descriptor_set_pool_;
    VulkanInstance const&                    vulkan_instance_;
    VkDevice const                           vk_logical_device_;
    bool                                     is_deprecated_{false};
};

} // namespace descriptor
} // namespace render
} // namespace vshade

namespace std
{
template <> struct hash<vshade::render::descriptor::VulkanDescriptorSet>
{
    std::size_t operator()(vshade::render::descriptor::VulkanDescriptorSet const& set) const
    {
        std::size_t result{0U};
        vshade::utils::hash_combine(result, set.getVkDescriptorSet());
        return result;
    }
};
} // namespace std

#endif // ENGINE_PLATFORM_RENDER_VULKAN_DESRIPTORS_VULKAN_DESCRIPTORS_SET_H