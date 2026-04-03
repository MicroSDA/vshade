#ifndef ENGINE_CORE_RENDER_BUFFERS_RENDER_COMMAND_QUEUE_H
#define ENGINE_CORE_RENDER_BUFFERS_RENDER_COMMAND_QUEUE_H

#include <engine/core/logs/loger.h>
#include <engine/core/utility/factory.h>

#include <array>
#include <memory>
#include <mutex>
#include <thread>

namespace vshade
{
namespace render
{

class RenderCommandQueue final : public utility::CRTPFactory<RenderCommandQueue>
{
    friend class utility::CRTPFactory<RenderCommandQueue>;

public:
    // can be changed on std::thread::hardware_concurrency()
    enum class Subgroup : std::uint8_t
    {
        _DEFUALT_ = 0U,
        _MAX_ENUM_
    };

    static constexpr std::size_t MAX_COMMANDS{1024U};
    static constexpr std::size_t BUFFER_SIZE{MAX_COMMANDS * 1024U * 5U}; // 5 MB

private:
    class ICommand
    {
    public:
        virtual void execute(std::uint32_t const frame_index) = 0;
        virtual ~ICommand()                                   = default;
    };

    template <typename Lambda> class Command : public ICommand
    {
    public:
        explicit Command(Lambda&& lambda) : lambda_(std::move(lambda))
        {
        }
        void execute(std::uint32_t const frame_index) override
        {
            lambda_(frame_index);
        }

    private:
        Lambda lambda_;
    };

    struct ThreadCommandQueue
    {
        std::unique_ptr<std::byte[]>        buffer{nullptr};
        std::array<ICommand*, MAX_COMMANDS> commands{};
        std::size_t                         count{0};
        std::size_t                         offset{0};
        std::mutex                          mutex;
    };

public:
    ~RenderCommandQueue();

    RenderCommandQueue(RenderCommandQueue const&)              = delete;
    RenderCommandQueue(RenderCommandQueue&&)                   = delete;
    RenderCommandQueue& operator=(RenderCommandQueue const&) & = delete;
    RenderCommandQueue& operator=(RenderCommandQueue&&) &      = delete;

    template <typename Lambda> void enqueue(Subgroup sub_queue, Lambda&& lambda)
    {
        if (sub_queue >= Subgroup::_MAX_ENUM_)
        {
            VSHADE_CORE_ERROR("Invalid thread index passed to RenderCommandQueue: {} (max: {})", static_cast<std::uint8_t>(sub_queue),
                              static_cast<std::uint8_t>(Subgroup::_MAX_ENUM_));
            return;
        }

        using CommandType = Command<Lambda>;
        static_assert(alignof(CommandType) <= alignof(std::max_align_t), "Bad command alignment!");

        auto&            queue = subqueues_[static_cast<std::uint8_t>(sub_queue)];
        std::scoped_lock lock(queue.mutex);

        if (queue.count >= MAX_COMMANDS || (queue.offset + sizeof(CommandType)) >= BUFFER_SIZE)
        {
            VSHADE_CORE_ERROR("RenderCommandQueue overflow in thread {} (count: {}, offset: {})", static_cast<int>(sub_queue), queue.count,
                              queue.offset);
            return;
        }

        void* ptr                     = queue.buffer.get() + queue.offset;
        auto* command                 = new (ptr) CommandType(std::forward<Lambda>(lambda));
        queue.commands[queue.count++] = command;
        queue.offset += sizeof(CommandType);
    }

    std::size_t getCount(Subgroup sub_queue) const
    {
        return subqueues_[static_cast<std::uint8_t>(sub_queue)].count;
    }

    void clearQueues();
    void executeAll(std::uint32_t const frame_index);
    // TODO: has to be in thread pool !!
    void executeAllAsync(std::uint32_t const frame_index);

private:
    explicit RenderCommandQueue();
    std::array<ThreadCommandQueue, static_cast<std::uint8_t>(Subgroup::_MAX_ENUM_)> subqueues_;
};

} // namespace render

} // namespace vshade

#endif // ENGINE_CORE_RENDER_BUFFERS_RENDER_COMMAND_QUEUE_H