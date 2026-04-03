#ifndef ENGINE_CORE_RENDER_BUFFERS_VERTEX_BUFER_H
#define ENGINE_CORE_RENDER_BUFFERS_VERTEX_BUFER_H

#include <engine/core/render/buffers/common.h>
#include <engine/core/render/render_command_buffer.h>
#include <engine/core/render/shader/shader.h>
#include <engine/core/utility/factory.h>

namespace vshade
{
namespace render
{
class VSHADE_API VertexBuffer : public utility::CRTPFactory<VertexBuffer>
{
    friend class utility::CRTPFactory<VertexBuffer>;
    using utility::CRTPFactory<VertexBuffer>::create;

public:
    class Layout final
    {
    public:
        enum class Usage : std::uint32_t
        {
            _PER_VERTEX_   = 0U,
            _PER_INSTANCE_ = 1U
        };

        struct Element
        {
            Element() = default;
            Element(std::string const& name, Shader::DataType shade_type)
                : name(name), shade_data_type{shade_type}, size{Shader::getDataTypeSize(shade_type)}
            {
            }
            std::string          name{"Undefined"};
            Shader::DataType     shade_data_type{Shader::DataType::_NONE_};
            std::uint32_t        size{0U};
            std::uint32_t        offset{0U};
            static std::uint32_t getComponentCount(Shader::DataType shade_data_type);
        };

        struct ElementLayout
        {
            Usage                usage{Usage::_PER_VERTEX_};
            std::vector<Element> elements;
        };

    public:
        Layout() = default;
        Layout(std::initializer_list<ElementLayout> const& elements);
        ~Layout() = default;

        std::uint32_t getStride(std::size_t layout) const
        {
            return strides_[layout];
        }
        std::uint32_t                     getCount() const;
        std::vector<ElementLayout> const& getElementLayout() const
        {
            return elements_;
        }

    private:
        void                       computeOffsetAndStride();
        std::vector<ElementLayout> elements_;
        std::vector<std::uint32_t> strides_;
    };

public:
    virtual ~VertexBuffer()                        = default;
    VertexBuffer(VertexBuffer const&)              = delete;
    VertexBuffer(VertexBuffer&&)                   = delete;
    VertexBuffer& operator=(VertexBuffer const&) & = delete;
    VertexBuffer& operator=(VertexBuffer&&) &      = delete;

    virtual void  setData(std::uint32_t const vertex_size, std::uint32_t const vertices_count, void const* data_ptr,
                          std::uint32_t const frame_index = 0U, std::uint32_t const offset = 0U) = 0;
    virtual void  resize(std::uint32_t const vertex_size, std::uint32_t const vertices_count)    = 0;
    virtual void  bind(std::shared_ptr<RenderCommandBuffer> redner_comand_buffer, std::uint32_t const frame_index, std::uint32_t const binding,
                       std::uint32_t const offset = 0U) const                                    = 0;
    std::uint32_t getSize() const
    {
        return data_size_;
    }
    virtual std::uint32_t getVerticesCount() const = 0;

    static std::shared_ptr<VertexBuffer> create(BufferUsage const usage, std::uint32_t const vertex_size, std::uint32_t const vertices_count,
                                                std::uint32_t const count = 1U, std::uint32_t const resize_threshold = 0U);

protected:
    explicit VertexBuffer(BufferUsage const usage, std::uint32_t const vertex_size, std::uint32_t const vertices_count,
                          std::uint32_t const count = 1U, std::uint32_t const resize_threshold = 0U);
    bool hasToBeResized(std::uint32_t const old_size, std::uint32_t const new_size, std::uint32_t const threshold);

    BufferUsage   usage_{BufferUsage::_GPU_};
    std::uint32_t data_size_{0U};
    std::uint32_t count_{0U};
    std::uint32_t vertices_count_{0U};
    std::uint32_t vertex_size_{0U};
    std::uint32_t resize_threshold_{0U};
};
} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_RENDER_BUFFERS_VERTEX_BUFER_H