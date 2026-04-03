#include "engine/core/render/buffers/vertex_buffer.h"
#include <engine/platforms/render/vulkan/buffers/vulkan_vertex_buffer.h>

vshade::render::VertexBuffer::VertexBuffer(BufferUsage const usage, std::uint32_t const vertex_size, std::uint32_t const vertices_count,
                                           std::uint32_t const count, std::uint32_t const resize_threshold)
    : usage_{usage}
    , data_size_{vertex_size * count}
    , vertex_size_{vertex_size}
    , vertices_count_{vertices_count}
    , count_{count}
    , resize_threshold_{resize_threshold}
{
}

std::shared_ptr<vshade::render::VertexBuffer> vshade::render::VertexBuffer::create(BufferUsage const usage, std::uint32_t const vertex_size,
                                                                                   std::uint32_t const vertices_count, std::uint32_t const count,
                                                                                   std::uint32_t const resize_threshold)
{
    switch (System::instance().getConfiguration().render_api)
    {
    case render::API::_VULKAN_:
        return utility::CRTPFactory<VertexBuffer>::create<VulkanVertexBuffer>(usage, vertex_size, vertices_count, count, resize_threshold);
    }
}

bool vshade::render::VertexBuffer::hasToBeResized(std::uint32_t const old_size, std::uint32_t const new_size, std::uint32_t const threshold)
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

vshade::render::VertexBuffer::Layout::Layout(std::initializer_list<ElementLayout> const& elements) : elements_(elements)
{

    computeOffsetAndStride();
}

std::uint32_t vshade::render::VertexBuffer::Layout::getCount() const
{
    std::uint32_t count{0U};
    for (auto const& element_layout : elements_)
    {
        count += element_layout.elements.size();
    }

    return count;
}

void vshade::render::VertexBuffer::Layout::computeOffsetAndStride()
{
    strides_.resize(elements_.size(), 0U);
    std::size_t total_size{0U};
    for (std::uint32_t layout_index{0U}; layout_index < elements_.size(); ++layout_index)
    {
        std::uint32_t offset{0U};

        for (auto& element : elements_[layout_index].elements)
        {
            element.offset = offset;
            offset += element.size;
            strides_[layout_index] += element.size;
            total_size += element.size;
        }
    }
}

std::uint32_t vshade::render::VertexBuffer::Layout::Element::getComponentCount(Shader::DataType shade_data_type)
{
    switch (shade_data_type)
    {
    case Shader::DataType::_FLOAT_:
        return 1U;
    case Shader::DataType::_FLOAT_2_:
        return 2U;
    case Shader::DataType::_FLOAT_3_:
        return 3U;
    case Shader::DataType::_FLOAT_4_:
        return 4U;
    case Shader::DataType::_MAT_3_:
        return 3U * 3U;
    case Shader::DataType::_MAT_4_:
        return 4U * 4U;
    case Shader::DataType::_INT_:
        return 1U;
    case Shader::DataType::_INT_2_:
        return 2U;
    case Shader::DataType::_INT_3_:
        return 3U;
    case Shader::DataType::_INT_4_:
        return 4U;
    case Shader::DataType::_BOOL_:
        return 1U;
    default:
        return 0U;
    }
}
