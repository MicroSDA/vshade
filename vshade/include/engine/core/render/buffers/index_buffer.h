#ifndef ENGINE_CORE_RENDER_BUFFERS_INDEX_BUFER_H
#define ENGINE_CORE_RENDER_BUFFERS_INDEX_BUFER_H

#include <engine/core/render/buffers/common.h>
#include <engine/core/render/render_command_buffer.h>
#include <engine/core/render/shader/shader.h>
#include <engine/core/utility/factory.h>

namespace vshade
{
namespace render
{
class VSHADE_API IndexBuffer : public utility::CRTPFactory<IndexBuffer>
{
    friend class utility::CRTPFactory<IndexBuffer>;
    using utility::CRTPFactory<IndexBuffer>::create;

public:
    virtual ~IndexBuffer()                       = default;
    IndexBuffer(IndexBuffer const&)              = delete;
    IndexBuffer(IndexBuffer&&)                   = delete;
    IndexBuffer& operator=(IndexBuffer const&) & = delete;
    IndexBuffer& operator=(IndexBuffer&&) &      = delete;

    virtual void  setData(std::uint32_t const size, void const* data, std::uint32_t const offset = 0U) = 0;
    virtual void  resize(std::uint32_t const size)                                                     = 0;
    virtual void  bind(std::shared_ptr<RenderCommandBuffer> redner_comand_buffer, std::uint32_t const frame_index, std::uint32_t const binding,
                       std::uint32_t const offset = 0U) const                                          = 0;
    std::uint32_t getSize() const
    {
        return data_size_;
    }

    static std::shared_ptr<IndexBuffer> create(BufferUsage const usage, std::uint32_t const data_size, std::uint32_t const resize_threshold = 0U,
                                               void const* data = nullptr);

protected:
    explicit IndexBuffer(BufferUsage const usage, std::uint32_t const data_size, std::uint32_t const resize_threshold, void const* data);
    bool hasToBeResized(std::uint32_t const old_size, std::uint32_t const new_size, std::uint32_t const threshold);

    BufferUsage   usage_;
    std::uint32_t data_size_{0U};
    std::uint32_t resize_threshold_{0U};
};
} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_RENDER_BUFFERS_INDEX_BUFER_H