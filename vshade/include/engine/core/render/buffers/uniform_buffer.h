#ifndef ENGINE_CORE_RENDER_BUFFERS_UNIFORM_BUFER_H
#define ENGINE_CORE_RENDER_BUFFERS_UNIFORM_BUFER_H

#include <engine/config/vshade_api.h>
#include <engine/core/render/buffers/common.h>
#include <engine/core/utility/factory.h>
#include <limits>

namespace vshade
{
namespace render
{
class VSHADE_API UniformBuffer : public utility::CRTPFactory<UniformBuffer>
{
    friend class utility::CRTPFactory<UniformBuffer>;
    using utility::CRTPFactory<UniformBuffer>::create;

public:
    virtual ~UniformBuffer()                         = default;
    UniformBuffer(UniformBuffer const&)              = delete;
    UniformBuffer(UniformBuffer&&)                   = delete;
    UniformBuffer& operator=(UniformBuffer const&) & = delete;
    UniformBuffer& operator=(UniformBuffer&&) &      = delete;

    virtual void setData(std::uint32_t const size, void const* data, std::uint32_t const frame_index = 0U, std::uint32_t const offset = 0U) = 0;
    virtual void resize(std::uint32_t const size)                                                                                           = 0;

    std::uint32_t getSize() const
    {
        return data_size_;
    }

    std::uint32_t getBindingIndex() const
    {
        return binding_index_;
    }

    static std::shared_ptr<UniformBuffer> create(BufferUsage const usage, std::uint32_t const binding_index, std::uint32_t const data_size,
                                                 std::uint32_t const count = 1U, std::uint32_t const resize_threshold = 0U);

protected:
    explicit UniformBuffer(BufferUsage const usage, std::uint32_t const binding_index, std::uint32_t const data_size, std::uint32_t const count,
                           std::uint32_t const resize_threshold);
    bool hasToBeResized(std::uint32_t const old_size, std::uint32_t const new_size, std::uint32_t const threshold);

    BufferUsage   usage_{BufferUsage::_GPU_};
    std::uint32_t data_size_{0U};
    std::uint32_t resize_threshold_{0U};
    std::uint32_t count_{0U};
    std::uint32_t binding_index_{std::numeric_limits<std::uint32_t>::max()};
};
} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_RENDER_BUFFERS_UNIFORM_BUFER_H