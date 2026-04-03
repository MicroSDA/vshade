#ifndef ENGINE_CORE_RENDER_RENDER_CONTEXT_H
#define ENGINE_CORE_RENDER_RENDER_CONTEXT_H

#include <engine/core/render/render_api.h>
#include <engine/core/utility/singleton.h>
#include <engine/core/render/buffers/render_command_queue.h>
#include <memory>

namespace vshade
{
namespace render
{
class VSHADE_API RenderContext : public utility::CRTPSingleton<RenderContext>
{
    friend class utility::CRTPSingleton<RenderContext>;

    // Hide functions
    using utility::CRTPSingleton<RenderContext>::create;
    using utility::CRTPSingleton<RenderContext>::destroy;

public:
    virtual ~RenderContext();
    RenderContext(RenderContext const&)              = delete;
    RenderContext(RenderContext&&)                   = delete;
    RenderContext& operator=(RenderContext const&) & = delete;
    RenderContext& operator=(RenderContext&&) &      = delete;

    virtual void shutDown() = 0;

     template <typename Command> void enqueDelete(Command&& command, RenderCommandQueue::Subgroup sub_group = RenderCommandQueue::Subgroup::_DEFUALT_)
    {
        delete_queue_->enqueue(sub_group, std::forward<Command>(command));
    }

    void deleteAllPendings(std::uint32_t const frame_index)
    {
        delete_queue_->executeAll(frame_index);
        delete_queue_->clearQueues();
    }
public:
    static void  create(API api);
    static void  destroy();
    virtual void intitialize() = 0;

protected:
    explicit RenderContext();
    std::shared_ptr<RenderCommandQueue>              delete_queue_;
};
} // namespace render
} // namespace vshade

#endif // ENGINE_CORE_RENDER_RENDER_CONTEXT_H