#ifndef ENGINE_PLATFORM_RENDER_VULKAN_DESRIPTORS_VULKAN_DESCRIPTOR_H
#define ENGINE_PLATFORM_RENDER_VULKAN_DESRIPTORS_VULKAN_DESCRIPTOR_H

#include <ankerl/unordered_dense.h>
#include <engine/core/utility/hash.h>
#include <engine/core/utility/singleton.h>
#include <engine/platforms/render/vulkan/descriptors/vulkan_descriptor_set.h>


namespace std
{
template <> struct hash<VkDescriptorBufferInfo>
{
    size_t operator()(VkDescriptorBufferInfo const& descriptor_buffer_info) const
    {
        std::size_t result{0U};

        vshade::utils::hash_combine(result, descriptor_buffer_info.buffer);
        vshade::utils::hash_combine(result, descriptor_buffer_info.range);
        vshade::utils::hash_combine(result, descriptor_buffer_info.offset);

        return result;
    }
};
template <> struct hash<VkDescriptorImageInfo>
{
    std::size_t operator()(VkDescriptorImageInfo const& descriptor_image_info) const
    {
        std::size_t result{0U};

        vshade::utils::hash_combine(result, descriptor_image_info.imageView);
        vshade::utils::hash_combine(result, descriptor_image_info.imageLayout);
        vshade::utils::hash_combine(result, descriptor_image_info.sampler);

        return result;
    }
};
} // namespace std

namespace vshade
{
namespace render
{
namespace descriptor
{
class VulkanDescriptorManager final : public utility::CRTPSingleton<VulkanDescriptorManager>
{
    friend class utility::CRTPSingleton<VulkanDescriptorManager>;

public:
    ~VulkanDescriptorManager();
    VulkanDescriptorManager(VulkanDescriptorManager const&)              = delete;
    VulkanDescriptorManager(VulkanDescriptorManager&&)                   = delete;
    VulkanDescriptorManager& operator=(VulkanDescriptorManager const&) & = delete;
    VulkanDescriptorManager& operator=(VulkanDescriptorManager&&) &      = delete;

    std::shared_ptr<VulkanDescriptorSet> reciveDescriptor(VulkanDescriptorSetLayout const& layout, VulkanDescriptorBindings const& buffer_bindings,
                                                          std::uint32_t const frame_index);
    void                                 resetAllDescripotrs(std::uint32_t const frame_index)
    {
        descriptor_pools_.at(frame_index);
        descriptor_sets_.at(frame_index);
    }
    void resetDepricated(std::uint32_t frame_index);

    template <typename T, typename... Args>
    std::shared_ptr<T> getPool(ankerl::unordered_dense::map<std::size_t, std::weak_ptr<T>>& resources, Args&... args)
    {
        std::size_t hash{0U};
        generateHash(hash, args...);

        auto resource{resources.find(hash)};
        if (resource != resources.end())
        {
            return resource->second.lock();
        }
        else
        {
            std::shared_ptr<T> newResource{std::make_shared<T>(vulkan_instance_, vk_logical_device_, args...)};
            resources.emplace(hash, newResource);
            return newResource;
        }
    }

    template <typename T, typename... Args>
    std::shared_ptr<T> getDescriptor(ankerl::unordered_dense::map<std::size_t, std::shared_ptr<T>>& resources, Args&... args)
    {
        std::size_t hash{0U};
        generateHash(hash, args...);

        auto resource{resources.find(hash)};
        if (resource != resources.end())
        {
            resource->second->setAsDepricated(false);
            return resource->second;
        }
        else
        {
            std::shared_ptr<T> newResource = std::make_shared<T>(vulkan_instance_, vk_logical_device_, args...);
            resources.emplace(hash, newResource);
            return newResource;
        }
    }

    template <typename T> void                   generateHash(std::size_t& seed, T const& value);
    template <typename T, typename... Args> void generateHash(size_t& seed, T const& first_arg, Args const&... args);

private:
    explicit VulkanDescriptorManager(std::uint32_t frames_in_flight);
    // Frame index -> hash (Descriptor layout) -> DescriptorPool
    std::vector<ankerl::unordered_dense::map<std::size_t, std::weak_ptr<VulkanDescriptorSetPool>>> descriptor_pools_;
    // Frame index -> hash (Descriptor layout + Buffers + ImageBuffers) -> DescriptorSet
    std::vector<ankerl::unordered_dense::map<std::size_t, std::shared_ptr<VulkanDescriptorSet>>> descriptor_sets_;
    std::uint32_t                                                                                frames_in_flight_{0U};

    VulkanInstance const& vulkan_instance_;
    VkDevice const        vk_logical_device_;
};

template <typename T> inline void VulkanDescriptorManager::generateHash(std::size_t& seed, T const& value)
{
    utils::hash_combine(seed, value);
}
template <> inline void VulkanDescriptorManager::generateHash<VulkanDescriptorBindings>(std::size_t& seed, VulkanDescriptorBindings const& value)
{
    for (auto& binding_set : value.buffers)
    {
        utils::hash_combine(seed, binding_set.first);

        for (auto& binding_element : binding_set.second)
        {
            utils::hash_combine(seed, binding_element.first);
            utils::hash_combine(seed, binding_element.second);
        }
    }

    for (auto& binding_set : value.images)
    {
        utils::hash_combine(seed, binding_set.first);

        for (auto& binding_element : binding_set.second)
        {
            utils::hash_combine(seed, binding_element.first);
            utils::hash_combine(seed, binding_element.second);
        }
    }
}
template <typename T, typename... Args> inline void VulkanDescriptorManager::generateHash(std::size_t& seed, T const& first_arg, Args const&... args)
{
    generateHash(seed, first_arg);

    generateHash(seed, args...);
}

} // namespace descriptor
} // namespace render
} // namespace vshade

#endif // ENGINE_PLATFORM_RENDER_VULKAN_DESRIPTORS_VULKAN_DESCRIPTOR_H