#include "engine/platforms/render/vulkan/descriptors/vulkan_descriptor_set_pool.h"

vshade::render::descriptor::VulkanDescriptorSetPool::VulkanDescriptorSetPool(VulkanInstance const& vulkan_instance, VkDevice const vk_logical_device,
                                                                             VulkanDescriptorSetLayout const& layout, std::uint32_t max_sets_per_pull)
    : vulkan_instance_{vulkan_instance}
    , vk_logical_device_{vk_logical_device}
    , vulkan_descriptor_set_layout_{layout}
    , max_sets_per_pull_{max_sets_per_pull}
{
    std::vector<VkDescriptorSetLayoutBinding> const& bindings{layout.getDescriptorSetLayoutBindings()};
    std::map<VkDescriptorType, std::uint32_t>        vk_descriptor_types_map;

    for (VkDescriptorSetLayoutBinding const& vk_descriptor_set_layout_binding : bindings)
    {
        vk_descriptor_types_map[vk_descriptor_set_layout_binding.descriptorType] += vk_descriptor_set_layout_binding.descriptorCount;
    }

    vk_descriptor_pool_sizes_.resize(vk_descriptor_types_map.size());

    std::vector<VkDescriptorPoolSize>::iterator pool_size_it = vk_descriptor_pool_sizes_.begin();

    for (auto& [vk_descriptor_type, count] : vk_descriptor_types_map)
    {
        pool_size_it->type            = vk_descriptor_type;
        pool_size_it->descriptorCount = count * max_sets_per_pull;
        ++pool_size_it;
    }

}
vshade::render::descriptor::VulkanDescriptorSetPool::~VulkanDescriptorSetPool()
{
    for (VkDescriptorPool vk_descriptor_pool : vk_descriptor_pools_)
    {
        vkDestroyDescriptorPool(vk_logical_device_, vk_descriptor_pool, vulkan_instance_.allocation_callbaks);
    }
}
VkDescriptorSet vshade::render::descriptor::VulkanDescriptorSetPool::allocateVkDescriptorSet()
{
    pool_index_ = findAvaliblePool(pool_index_);

    VkDescriptorSetLayout vk_descriptor_set_layout{vulkan_descriptor_set_layout_.getVkDescriptorSetLayout()};

    VkDescriptorSetAllocateInfo vk_descriptor_set_allocate_anfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    vk_descriptor_set_allocate_anfo.pNext                       = VK_NULL_HANDLE;
    vk_descriptor_set_allocate_anfo.descriptorPool              = vk_descriptor_pools_[pool_index_];
    vk_descriptor_set_allocate_anfo.descriptorSetCount          = 1U;
    vk_descriptor_set_allocate_anfo.pSetLayouts                 = &vk_descriptor_set_layout;

    VkDescriptorSet vk_descriptor_set{VK_NULL_HANDLE};

    if (vkAllocateDescriptorSets(vk_logical_device_, &vk_descriptor_set_allocate_anfo, &vk_descriptor_set) != VK_SUCCESS)
    {
        VSHADE_CORE_WARNING("Couldn't allocate descriptor set !")
        return VK_NULL_HANDLE;
    }

    vk_utils::setDebugObjectName<VK_OBJECT_TYPE_DESCRIPTOR_SET>(vulkan_instance_.instance, "Vulkan descriptor set", vk_logical_device_,
                                                                vk_descriptor_set);

    // So we have found some index and allocated pool for it, let's increment sets count of this pool
    ++vk_descriptor_pools_sets_count_[pool_index_];
    // Add mapping
    vk_descriptor_pool_mapping_.emplace(vk_descriptor_set, pool_index_);

    return vk_descriptor_set;
}
std::uint32_t vshade::render::descriptor::VulkanDescriptorSetPool::findAvaliblePool(std::uint32_t search_index)
{
    // Create a new pool
    if (vk_descriptor_pools_.size() <= search_index)
    {
        VkDescriptorPoolCreateInfo vk_descriptor_pool_create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        vk_descriptor_pool_create_info.pNext         = VK_NULL_HANDLE;
        vk_descriptor_pool_create_info.flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        vk_descriptor_pool_create_info.maxSets       = max_sets_per_pull_;
        vk_descriptor_pool_create_info.poolSizeCount = static_cast<std::uint32_t>(vk_descriptor_pool_sizes_.size());
        vk_descriptor_pool_create_info.pPoolSizes    = vk_descriptor_pool_sizes_.data();

        VkDescriptorPool vk_descriptor_pool{VK_NULL_HANDLE};

        // Create the Vulkan descriptor pool
        VkResult result{
            vkCreateDescriptorPool(vk_logical_device_, &vk_descriptor_pool_create_info, vulkan_instance_.allocation_callbaks, &vk_descriptor_pool)};

        if (result != VK_SUCCESS)
        {
            return 0U;
        }

        vk_utils::setDebugObjectName<VK_OBJECT_TYPE_DESCRIPTOR_POOL>(vulkan_instance_.instance, "Vulkan descriptor pool", vk_logical_device_,
                                                                     vk_descriptor_pool);
        // Store internally the Vulkan handle
        vk_descriptor_pools_.push_back(vk_descriptor_pool);

        vk_descriptor_pools_sets_count_.push_back(0U);

        return search_index;
    }
    else if (vk_descriptor_pools_sets_count_.at(search_index) < max_sets_per_pull_)
    {
        return search_index;
    }

    // Increment pool index
    return findAvaliblePool(++search_index);
}