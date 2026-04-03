#ifndef ENGINE_CORE_RENDER_RENDER_COMMAND_BUFFER_H
#define ENGINE_CORE_RENDER_RENDER_COMMAND_BUFFER_H

#include <array>
#include <engine/config/system.h>
#include <engine/core/utility/factory.h>
#include <mutex>
#include <vector>

namespace vshade
{
namespace render
{
class VSHADE_API RenderCommandBuffer : public utility::CRTPFactory<RenderCommandBuffer>
{
    friend class utility::CRTPFactory<RenderCommandBuffer>;

    // Hide functions
    using utility::CRTPFactory<RenderCommandBuffer>::create;

public:
    enum class Type
    {
        _SECONDARY_ = 1,
        _PRIMARY_
    };
    enum class Family
    {
        _PRESENT_,
        _GRAPHIC_,
        _TRANSFER_,
        _COMPUTE_,
        _FAMILY_MAX_ENUM_
    };

public:
    virtual ~RenderCommandBuffer()                               = default;
    RenderCommandBuffer(RenderCommandBuffer const&)              = delete;
    RenderCommandBuffer(RenderCommandBuffer&&)                   = delete;
    RenderCommandBuffer& operator=(RenderCommandBuffer const&) & = delete;
    RenderCommandBuffer& operator=(RenderCommandBuffer&&) &      = delete;

    virtual void        begin(std::uint32_t const frame_index = 0U)                                                                            = 0;
    virtual void        end(std::uint32_t const frame_index = 0U)                                                                              = 0;
    virtual void        submit(std::uint32_t const frame_index = 0U, std::chrono::nanoseconds const timeout = std::chrono::nanoseconds::max()) = 0;
    virtual std::mutex& getQueueMutex(Family const family)                                                                                     = 0;
    static std::shared_ptr<RenderCommandBuffer> create(Type const type = Type::_PRIMARY_, Family const family = Family::_GRAPHIC_,
                                                       std::uint32_t const count = 1U);
    bool                                        isRecordered(std::uint32_t const frame_index) const
    {
        return is_recordered_[frame_index];
    }

    bool isInRecorderStage(std::uint32_t const frame_index) const
    {
        return is_in_recorder_stage_[frame_index];
    }

    void setAsUnrecordered(std::uint32_t const frame_index)
    {
        is_recordered_[frame_index] = false;
    }

protected:
    explicit RenderCommandBuffer(Type const type = Type::_PRIMARY_, Family const family = Family::_GRAPHIC_, std::uint32_t const count = 1U);
    Type const        type_;
    Family const      family_;
    std::vector<bool> is_recordered_;
    std::vector<bool> is_in_recorder_stage_;

    static inline std::array<std::mutex, 20U> mutexs_;
};
} // namespace render
} // namespace vshade
#endif // ENGINE_CORE_RENDER_RENDER_COMMAND_BUFFER_H