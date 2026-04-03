#ifndef ENGINE_CORE_APPLICATION_APPLICATION_H
#define ENGINE_CORE_APPLICATION_APPLICATION_H

#include <engine/config/system.h>
#include <engine/core/events/event.h>
#include <engine/core/layer/layer_manager.h>
#include <engine/core/logs/loger.h>
#include <engine/core/render/render.h>
#include <engine/core/serialalizing/file_manager.h>
#include <engine/core/threads/thread_pool.h>
#include <engine/core/time/frame_time.h>
#include <engine/core/utility/singleton.h>
#include <engine/core/window/window.h>
#include <memory>

int main(int argc, char* argv[]);

namespace vshade
{
namespace event
{
class EventManager;
}

/// @brief
class VSHADE_API Application : public utility::CRTPSingleton<Application>
{
    friend class utility::CRTPSingleton<Application>;
    friend class event::EventManager;

    friend int ::main(int argc, char* argv[]);

public:
    virtual ~Application()                       = default;
    Application(Application const&)              = delete;
    Application(Application&&)                   = delete;
    Application& operator=(Application const&) & = delete;
    Application& operator=(Application&&) &      = delete;

    virtual void onCreate()  = 0;
    virtual void onDestroy() = 0;
    virtual void onUpdate()  = 0;

    void quitRequest();

    std::string getRootDirectory() const
    {
        return root_directory_;
    }

protected:
    virtual bool onEvent(std::shared_ptr<event::Event> const event) { return false; }
private:
    void initialize();
    void launch();
    void whileRunning();
    void terminate();
    void onEvent_(std::shared_ptr<event::Event> const event);

private:
    bool              is_quit_requested_{false};
    std::string const root_directory_;
    time::FrameTimer  frame_timer_;

protected:
    explicit Application(std::string const& root_directory_);
};

void createApplication();
void destroyApplication();
} // namespace vshade

#endif ENGINE_CORE_APPLICATION_APPLICATION_H