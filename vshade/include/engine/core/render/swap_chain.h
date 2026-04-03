#ifndef ENGINE_CORE_RENDER_SWAP_CHAIN_H
#define ENGINE_CORE_RENDER_SWAP_CHAIN_H

#include <engine/core/events/event.h>
#include <engine/core/render/buffers/frame_buffer.h>
#include <engine/core/render/pipeline.h>
#include <engine/core/utility/singleton.h>

namespace vshade
{
namespace render
{
class VSHADE_API SwapChain : public utility::CRTPSingleton<SwapChain>
{
    friend class utility::CRTPSingleton<SwapChain>;

    // Hide functions
    using utility::CRTPSingleton<SwapChain>::create;
    using utility::CRTPSingleton<SwapChain>::destroy;

protected:
    inline static std::uint64_t packResoulution(std::uint32_t const width, std::uint32_t const height)
    {
        return (std::uint64_t(width) << 32U) | height;
    }
    inline static std::pair<std::uint32_t, std::uint32_t> unpacResoulution(std::uint64_t const packed)
    {
        return std::pair<std::uint32_t, std::uint32_t>{std::uint32_t(packed >> 32U), std::uint32_t(packed & 0xFFFFFFFF)};
    }

public:
    virtual ~SwapChain()                     = default;
    SwapChain(SwapChain const&)              = delete;
    SwapChain(SwapChain&&)                   = delete;
    SwapChain& operator=(SwapChain const&) & = delete;
    SwapChain& operator=(SwapChain&&) &      = delete;

    virtual std::pair<std::uint32_t, std::uint32_t> createFrame(std::uint32_t const width, std::uint32_t const height,
                                                                std::uint32_t const frames_in_flyght, bool const v_sync)                   = 0;
    virtual void                                    createSurface(void* window_native_handle)                                              = 0;
    virtual void                                    beginFrame(std::uint32_t const frame_index)                                            = 0;
    virtual void                                    endFrame(std::uint32_t const frame_index)                                              = 0;
    virtual bool                                    present(std::chrono::nanoseconds const timeout, std::uint32_t const frame_index)       = 0;
    virtual void                                    resize(std::uint32_t const width, std::uint32_t const height)                          = 0;
    virtual bool                                    onResizeEvent(std::shared_ptr<event::WindowResize> const event)                        = 0;
    virtual void                            resolveIntoSwapChain(std::shared_ptr<Texture2D const> const texture, std::uint32_t const frame_index,
                                                                 Texture2D::Filtration const filtration = Texture2D::Filtration::_LINEAR_) = 0;
    std::pair<std::uint32_t, std::uint32_t> getResolution() const
    {
        return unpacResoulution(swaph_chain_resolution_.load(std::memory_order_seq_cst));
    }

    static void create();
    static void destroy();

    double getPresentDeltaTime() const
    {
        return present_delta_time_;
    }

protected:
    explicit SwapChain() = default;
    bool                                      v_sync_{false};
    std::uint32_t                             frames_in_flyght_{1U};
    std::atomic<bool>                         is_resize_requested_{false};
    std::vector<std::shared_ptr<FrameBuffer>> frame_buffers_;
    std::atomic<uint64_t>                     swaph_chain_resolution_;

    double present_delta_time_{1.0};
};
} // namespace render
} // namespace vshade
#endif // ENGINE_CORE_RENDER_SWAP_CHAIN_H