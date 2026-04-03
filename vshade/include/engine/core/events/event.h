#ifndef ENGINE_CORE_EVENTS_EVENT_H
#define ENGINE_CORE_EVENTS_EVENT_H

#include <cstdint>
#include <engine/core/events/key_codes.h>
#include <functional>

#define BIT(x) (1 << x)

namespace vshade
{
namespace event
{
class Event
{
public:
    enum class Type : int
    {
        _NONE_ = 0,
        _WINDOW_CLOSE_,
        _WINDOW_RESIZE_,
        _WINDOW_FOCUSED_,
        _WINDOW_UNFOCUSED_,
        _WINDOW_MOVED_,
        _APP_TICK_,
        _APP_UPDATE_,
        _APP_RENDER_,
        _KEY_PRESSED_,
        _KEY_RELEASED_,
        _KEY_TYPED_,
        _MOUSE_BUTTON_PRESSED_,
        _MOUSE_BUTTON_RELEASED_,
        _MOUSE_MOVED_,
        _MOUSE_SCROLLED_
    };
    enum Category : int
    {
        _NONE_              = 0,
        _APPLICATION_EVENT_ = BIT(0),
        _INPUT_EVENT_       = BIT(1),
        _KEYBOARD_EVENT_    = BIT(2),
        _MOUSE_EVENT_       = BIT(3),
        _MOUSE_BUTTON_EVENT = BIT(4),
        _USER_EVENT_        = BIT(5)
    };

    explicit Event() = default;
    virtual ~Event() = default;

    virtual Type        getType() const          = 0;
    virtual char const* getName() const          = 0;
    virtual int         getCategoryFlags() const = 0;

    inline bool isInCategory(Category category) const
    {
        return getCategoryFlags() & static_cast<int>(category);
    }
    template <typename U> U& as()
    {
        static_assert(std::is_base_of_v<Event, U>, "U must be derived from T");
        return static_cast<U&>(*this);
    }

    template <typename U> U const& as() const
    {
        static_assert(std::is_base_of_v<Event, U>, "U must be derived from T");
        return static_cast<U const&>(*this);
    }

    std::function<void(Event&)> event_callback; // Why public ?
};

#ifndef EVENT_TYPE
#define EVENT_TYPE(type)                                                                                                                             \
    inline static Event::Type getStaticType()                                                                                                        \
    {                                                                                                                                                \
        return Event::Type::type;                                                                                                                    \
    }                                                                                                                                                \
    inline virtual Event::Type getType() const override                                                                                              \
    {                                                                                                                                                \
        return getStaticType();                                                                                                                      \
    }                                                                                                                                                \
    inline virtual char const* getName() const override                                                                                              \
    {                                                                                                                                                \
        return #type;                                                                                                                                \
    }
#endif

#ifndef EVENT_CATEGORY
#define EVENT_CATEGORY(category)                                                                                                                     \
    inline virtual int getCategoryFlags() const override                                                                                             \
    {                                                                                                                                                \
        return static_cast<int>(category);                                                                                                           \
    }
#endif // EVENT_CATEGORY

class WindowClose final : public Event
{
public:
    EVENT_TYPE(_WINDOW_CLOSE_)
    EVENT_CATEGORY(_APPLICATION_EVENT_)
    explicit WindowClose() = default;
    virtual ~WindowClose() = default;
};
class WindowResize final : public Event
{
public:
    EVENT_TYPE(_WINDOW_RESIZE_)
    EVENT_CATEGORY(_APPLICATION_EVENT_)
    explicit WindowResize(std::uint32_t const width, std::uint32_t const height) : width_{width}, height_{height}
    {
    }
    virtual ~WindowResize() = default;

    std::uint32_t getWidth() const
    {
        return width_;
    }
    std::uint32_t getHeight() const
    {
        return height_;
    }

private:
    std::uint32_t const width_{0};
    std::uint32_t const height_{0};
};

class KeyEvent : public Event
{
public:
    EVENT_CATEGORY(_KEYBOARD_EVENT_ | _INPUT_EVENT_)
    explicit KeyEvent(KeyCode const key_code) : key_code_{key_code}
    {
    }
    virtual ~KeyEvent() = default;

    KeyCode GetKeyCode() const
    {
        return key_code_;
    }

protected:
    KeyCode const key_code_;
};
class KeyPressedEvent : public KeyEvent
{
public:
    EVENT_TYPE(_KEY_PRESSED_)
    explicit KeyPressedEvent(KeyCode const key_code, std::uint16_t const repeat_count) : KeyEvent{key_code}, repeat_count_{repeat_count}
    {
    }
    virtual ~KeyPressedEvent() = default;

    std::uint16_t getRepeatCount() const
    {
        return repeat_count_;
    }

protected:
    std::uint16_t const repeat_count_;
};

class KeyReleasedEvent : public KeyEvent
{
public:
    EVENT_TYPE(_KEY_RELEASED_)
    explicit KeyReleasedEvent(KeyCode const key_code) : KeyEvent{key_code}
    {
    }
    virtual ~KeyReleasedEvent() = default;
};

class KeyTypedEvent : public KeyEvent
{
public:
    EVENT_TYPE(_KEY_TYPED_)
    KeyTypedEvent(KeyCode const key_code) : KeyEvent{key_code}
    {
    }
    virtual ~KeyTypedEvent() = default;
};

class MouseEvent : public Event
{
public:
    EVENT_CATEGORY(_MOUSE_EVENT_ | _INPUT_EVENT_);
    explicit MouseEvent() = default;
    virtual ~MouseEvent() = default;
};

class MouseMovedEvent : public MouseEvent
{
public:
    EVENT_TYPE(_MOUSE_MOVED_);
    explicit MouseMovedEvent(double const x, double const y) : mouse_x_{x}, mouse_y_{y}
    {
    }
    virtual ~MouseMovedEvent() = default;

    double getX() const
    {
        return mouse_x_;
    }
    double getY() const
    {
        return mouse_y_;
    }

private:
    double const mouse_x_, mouse_y_;
};

class MouseScrolledEvent : public MouseEvent
{
public:
    EVENT_TYPE(_MOUSE_SCROLLED_);
    explicit MouseScrolledEvent(double const x_offset, double const y_offset) : x_offset_(x_offset), y_offset_(y_offset)
    {
    }
    virtual ~MouseScrolledEvent() = default;

    double getXOffset() const
    {
        return x_offset_;
    }
    double getYOffset() const
    {
        return y_offset_;
    }

private:
    double const x_offset_, y_offset_;
};

class MouseButtonEvent : public MouseEvent
{
public:
    EVENT_CATEGORY(_MOUSE_EVENT_ | _INPUT_EVENT_ | _MOUSE_BUTTON_EVENT);
    explicit MouseButtonEvent(MouseButtonCode const button_code) : button_code_{button_code}
    {
    }
    virtual ~MouseButtonEvent() = default;

    MouseButtonCode GetMouseButton() const
    {
        return button_code_;
    }

protected:
    MouseButtonCode const button_code_;
};

class MouseButtonPressedEvent : public MouseButtonEvent
{
public:
    EVENT_TYPE(_MOUSE_BUTTON_PRESSED_)
    explicit MouseButtonPressedEvent(MouseButtonCode const button_code) : MouseButtonEvent{button_code}
    {
    }
    virtual ~MouseButtonPressedEvent() = default;
};

class MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
    EVENT_TYPE(_MOUSE_BUTTON_RELEASED_)
    explicit MouseButtonReleasedEvent(MouseButtonCode const button_code) : MouseButtonEvent{button_code}
    {
    }
    virtual ~MouseButtonReleasedEvent() = default;
};
} // namespace event
} // namespace vshade

#endif // ENGINE_CORE_EVENTS_EVENT_H