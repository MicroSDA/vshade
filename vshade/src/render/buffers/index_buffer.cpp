#include "engine/core/render/buffers/index_buffer.h"
#include <engine/platforms/render/vulkan/buffers/vulkan_index_buffer.h>

vshade::render::IndexBuffer::IndexBuffer(BufferUsage const usage, std::uint32_t const data_size, std::uint32_t const resize_threshold,
                                         void const* data)
    : usage_{usage}, data_size_{data_size}, resize_threshold_{resize_threshold}
{
}

std::shared_ptr<vshade::render::IndexBuffer> vshade::render::IndexBuffer::create(BufferUsage const usage, std::uint32_t const data_size,
                                                                                 std::uint32_t const resize_threshold, void const* data)
{
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        return utility::CRTPFactory<IndexBuffer>::create<VulkanIndexBuffer>(usage, data_size, resize_threshold, data);
    }
}

bool vshade::render::IndexBuffer::hasToBeResized(std::uint32_t const old_size, std::uint32_t const new_size, std::uint32_t  const threshold)
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