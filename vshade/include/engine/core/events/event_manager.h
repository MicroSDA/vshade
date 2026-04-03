#ifndef ENGINE_CORE_EVENTS_EVENT_MANAGER_H
#define ENGINE_CORE_EVENTS_EVENT_MANAGER_H

#include <engine/core/application/application.h>
#include <engine/core/events/event.h>
#include <engine/core/events/input.h>
#include <engine/core/utility/singleton.h>
#include <functional>
#include <mutex>
#include <queue>

namespace vshade
{
namespace event
{
class VSHADE_API EventManager final : public utility::CRTPSingleton<EventManager>
{
    friend class utility::CRTPSingleton<EventManager>;

    template <typename TEvent> using EventCallback = std::function<bool(std::shared_ptr<TEvent> const)>;

    // Hide functions
    using utility::CRTPSingleton<EventManager>::create;
    using utility::CRTPSingleton<EventManager>::destroy;

public:
    virtual ~EventManager()                        = default;
    EventManager(EventManager const&)              = delete;
    EventManager(EventManager&&)                   = delete;
    EventManager& operator=(EventManager const&) & = delete;
    EventManager& operator=(EventManager&&) &      = delete;

    template <typename TEvent, typename... Args> static void pushEvent(Args&&... args)
    {
        static_assert(std::is_base_of_v<Event, TEvent>, "TEvent must be derived from Event");

        auto task = [args_tuple = std::make_tuple(std::forward<Args>(args)...)]() mutable
        {
            std::shared_ptr<TEvent> event =
                std::apply([](auto&&... unpacked_args) { return std::make_shared<TEvent>(std::forward<decltype(unpacked_args)>(unpacked_args)...); },
                           std::move(args_tuple));

            Application::instance().onEvent_(event);
        };
        {
            std::scoped_lock lock(events_queue_mutex_);
            events_pool_.push(std::move(task));
        }
    }
    template <typename TEvent> static void dispatchEvent(EventCallback<TEvent> callback, std::shared_ptr<Event> const event)
    {
        if (event->getType() == TEvent::getStaticType())
        {
            callback(std::static_pointer_cast<TEvent>(event));
        }
    }

    static void create();
    static void destroy();

public:
    void processEvents();

private:
    static inline std::queue<std::function<void()>> events_pool_;
    static inline std::mutex                        events_queue_mutex_;

protected:
    explicit EventManager() = default;
};
} // namespace event
} // namespace vshade

#endif // ENGINE_CORE_EVENTS_EVENT_MANAGER_H