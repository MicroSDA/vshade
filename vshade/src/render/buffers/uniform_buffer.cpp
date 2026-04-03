#include "engine/core/render/buffers/uniform_buffer.h"
#include <engine/platforms/render/vulkan/buffers/vulkan_uniform_buffer.h>

vshade::render::UniformBuffer::UniformBuffer(BufferUsage const usage, std::uint32_t const binding_index, std::uint32_t const data_size,
                                             std::uint32_t const count, std::uint32_t const resize_threshold)
    : usage_{usage}, binding_index_{binding_index}, data_size_{data_size}, count_{count}, resize_threshold_{resize_threshold}
{
}

std::shared_ptr<vshade::render::UniformBuffer> vshade::render::UniformBuffer::create(BufferUsage const usage, std::uint32_t const binding_index,
                                                                                     std::uint32_t const data_size, std::uint32_t const count,
                                                                                     std::uint32_t const resize_threshold)
{
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        return utility::CRTPFactory<UniformBuffer>::create<VulkanUniformBuffer>(usage, binding_index, data_size, count, resize_threshold);
    }
    return std::shared_ptr<UniformBuffer>();
}

bool vshade::render::UniformBuffer::hasToBeResized(std::uint32_t const old_size, std::uint32_t const new_size, std::uint32_t threshold)
{
    // Checks if the new size is larger than the old size
    if (old_size < new_size)
    {
        return true;
    }
    // Checks if the increase in size is more than the given threshold percentage of old size
    else if (old_size > (new_size + ((old_size / 100U) * threshold)))
    {
        return true;
    }

    return false;
}