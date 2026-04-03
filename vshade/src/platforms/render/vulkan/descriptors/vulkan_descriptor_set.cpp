#include "engine/platforms/render/vulkan/descriptors/vulkan_descriptor_set.h"

vshade::render::descriptor::VulkanDescriptorSet::VulkanDescriptorSet(VulkanInstance const& vulkan_instance, VkDevice const vk_logical_device,
                                                                     VulkanDescriptorSetLayout const&         layout,
                                                                     std::shared_ptr<VulkanDescriptorSetPool> pool,
                                                                     VulkanDescriptorBindings const&          descriptor_bindings)
    : vulkan_instance_(vulkan_instance)
    , vk_logical_device_(vk_logical_device)
    , vulkan_descriptor_set_layout_(layout)
    , vulkane_descriptor_set_pool_(pool)
    , vk_descriptor_set_(pool->allocateVkDescriptorSet())
{
    if (!descriptor_bindings.buffers.empty() || !descriptor_bindings.images.empty())
    {
        update(descriptor_bindings.buffers, descriptor_bindings.images);
    }
}

void vshade::render::descriptor::VulkanDescriptorSet::update(DescriptorsBindingsMap<VkDescriptorBufferInfo> const& buffer_infos,
                                                             DescriptorsBindingsMap<VkDescriptorImageInfo> const&  image_infos)
{
    std::vector<VkWriteDescriptorSet> vk_write_descriptor_sets;

    // Iterate over all buffer bindings
    for (auto& [binding, set] : buffer_infos)
    {
        for (auto& [element, buffer] : set)
        {
            if (VkDescriptorSetLayoutBinding const* layout_binding{vulkan_descriptor_set_layout_.getDescriptorSetLayoutBinding(binding)})
            {
                VkWriteDescriptorSet vk_write_descriptor_set{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                vk_write_descriptor_set.pNext            = VK_NULL_HANDLE;
                vk_write_descriptor_set.dstSet           = vk_descriptor_set_;
                vk_write_descriptor_set.dstBinding       = static_cast<std::uint32_t>(binding);
                vk_write_descriptor_set.dstArrayElement  = static_cast<std::uint32_t>(element);
                vk_write_descriptor_set.descriptorCount  = 1U;
                vk_write_descriptor_set.descriptorType   = layout_binding->descriptorType;
                vk_write_descriptor_set.pImageInfo       = VK_NULL_HANDLE;
                vk_write_descriptor_set.pBufferInfo      = &buffer;
                vk_write_descriptor_set.pTexelBufferView = VK_NULL_HANDLE;

                vk_write_descriptor_sets.push_back(vk_write_descriptor_set);
            }
        }
    }
    // Iterate over all buffer bindings
    for (auto& [binding, set] : image_infos)
    {
        for (auto& [element, buffer] : set)
        {
            if (VkDescriptorSetLayoutBinding const* layout_binding{vulkan_descriptor_set_layout_.getDescriptorSetLayoutBinding(binding)})
            {

                VkWriteDescriptorSet vk_write_descriptor_set{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                vk_write_descriptor_set.pNext            = VK_NULL_HANDLE;
                vk_write_descriptor_set.dstSet           = vk_descriptor_set_;
                vk_write_descriptor_set.dstBinding       = static_cast<std::uint32_t>(binding);
                vk_write_descriptor_set.dstArrayElement  = static_cast<std::uint32_t>(element);
                vk_write_descriptor_set.descriptorCount  = 1U;
                vk_write_descriptor_set.descriptorType   = layout_binding->descriptorType;
                vk_write_descriptor_set.pImageInfo       = &buffer;
                vk_write_descriptor_set.pBufferInfo      = VK_NULL_HANDLE;
                vk_write_descriptor_set.pTexelBufferView = VK_NULL_HANDLE;

                vk_write_descriptor_sets.push_back(vk_write_descriptor_set);
            }
        }
    }

    vkUpdateDescriptorSets(vk_logical_device_, static_cast<std::uint32_t>(vk_write_descriptor_sets.size()), vk_write_descriptor_sets.data(), 0U,
                           VK_NULL_HANDLE);
}
