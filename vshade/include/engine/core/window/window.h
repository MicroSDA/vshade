#ifndef ENGINE_CORE_WINDOW_WINDOW_H
#define ENGINE_CORE_WINDOW_WINDOW_H

#include <engine/config/system.h>
#include <engine/config/vshade_api.h>
#include <engine/core/events/event.h>
#include <engine/core/render/render_api.h>
#include <engine/core/render/swap_chain.h>
#include <engine/core/utility/singleton.h>
#include <glm/glm/glm.hpp>

namespace vshade
{
namespace window
{
struct Properties
{
    // TODO: Read from config file
    std::string   title{"Window"};     // Title of the window.
    std::uint32_t width{1920};         // Width of the window.
    std::uint32_t height{1200};        // Height of the window.
    bool          full_screen{false};  // Whether the window is fullscreen.
    bool          v_sync{false};       // Whether VSync is enabled.
    bool          no_title_bar{false}; // Disable title bar.
};

class VSHADE_API Window : public utility::CRTPSingleton<Window>
{
    friend class utility::CRTPSingleton<Window>;

    // Hide functions
    using utility::CRTPSingleton<Window>::create;
    using utility::CRTPSingleton<Window>::destroy;

public:
    virtual ~Window()                  = default;
    Window(Window const&)              = delete;
    Window(Window&&)                   = delete;
    Window& operator=(Window const&) & = delete;
    Window& operator=(Window&&) &      = delete;

    bool onCloseEvent(std::shared_ptr<event::WindowClose> const event);
    bool onResizeEvent(std::shared_ptr<event::WindowResize> const event);

    std::uint32_t getWidth();
    std::uint32_t getHeight();

    virtual void  processEvents()          = 0;
    virtual void* getWindowNativeHanlder() = 0;

    virtual void       setWindowOpacity(float const alpha, void* window = nullptr)           = 0;
    virtual void       setWindowPosition(glm::ivec2 const& position, void* window = nullptr) = 0;
    virtual void       moveWindow(glm::ivec2 const& position, void* window = nullptr)        = 0;
    virtual glm::ivec2 getWindowPosition(void* window = nullptr) const                       = 0;
    virtual glm::ivec2 getCursorPosition(void* window = nullptr) const                       = 0;
    virtual void       minimazeWindow(void* window = nullptr)                                = 0;
    virtual void       maximizeWindow(void* window = nullptr)                                = 0;
    virtual void       closeWindow(void* window = nullptr)                                   = 0;
    virtual bool       isShouldClose(void* window = nullptr) const                           = 0;
    static void        create(Properties const& properties);
    static void        destroy();

protected:
    explicit Window(Properties const& properties);
    virtual void initWindowEvents() = 0;
    Properties   properties_;
};
} // namespace window
} // namespace vshade

#endif // ENGINE_CORE_WINDOW_WINDOW_H