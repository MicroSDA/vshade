#include "engine/platforms/render/vulkan/descriptors/vulkan_descriptor.h"

vshade::render::descriptor::VulkanDescriptorManager::VulkanDescriptorManager(std::uint32_t frames_in_flight)
    : frames_in_flight_{frames_in_flight}
    , vulkan_instance_{RenderContext::instance().as<VulkanRenderContext>().getVulkanInstance()}
    , vk_logical_device_{RenderContext::instance().as<VulkanRenderContext>().getLogicalDevice()->getVkDevice()}

{
    descriptor_pools_.resize(frames_in_flight);
    descriptor_sets_.resize(frames_in_flight);
}


vshade::render::descriptor::VulkanDescriptorManager::~VulkanDescriptorManager()
{
    descriptor_pools_.clear();
	descriptor_sets_.clear();
}

std::shared_ptr<vshade::render::descriptor::VulkanDescriptorSet>
vshade::render::descriptor::VulkanDescriptorManager::reciveDescriptor(VulkanDescriptorSetLayout const& layout,
                                                                      VulkanDescriptorBindings const& buffer_bindings, std::uint32_t const frame_index)
{
    auto descriptor_pool = getPool(descriptor_pools_.at(frame_index), layout);
    return getDescriptor(descriptor_sets_.at(frame_index), layout, descriptor_pool, buffer_bindings);
}

void vshade::render::descriptor::VulkanDescriptorManager::resetDepricated(std::uint32_t const frame_index)
{
    // It's wrong behavior, wee need to remove descriptor only if resource (texture, buffer, etc.) has been destroed !
    for (auto descriptor{descriptor_sets_[frame_index].begin()}; descriptor != descriptor_sets_[frame_index].end();)
    {
        if (descriptor->second->isDepricated())
        {
            // SHADE_CORE_DEBUG("Removing descriptor set = {0}", descriptor->first);
            descriptor_sets_[frame_index].erase(descriptor++);
        }
        else
        {
            descriptor->second->setAsDepricated(true);
            ++descriptor;
        }
    }

    for (auto pool{descriptor_pools_[frame_index].begin()}; pool != descriptor_pools_[frame_index].end();)
    {
        if (pool->second.expired())
        {
            // SHADE_CORE_DEBUG("Removing descriptor pool = {0}", pool->first);
            descriptor_pools_[frame_index].erase(pool++);
        }
        else
        {
            ++pool;
        }
    }
}
