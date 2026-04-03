#include "engine/core/events/input.h"
#include <engine/core/window/window.h>

#include <GLFW/glfw3.h>

bool vshade::event::Input::isKeyPressed(KeyCode const key_code)
{
    auto* window = static_cast<GLFWwindow*>(window::Window::instance().getWindowNativeHanlder());
    auto  state  = glfwGetKey(window, static_cast<std::int32_t>(key_code));
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool vshade::event::Input::isMouseButtonPressed(MouseButtonCode const button_code)
{
    auto* window = static_cast<GLFWwindow*>(window::Window::instance().getWindowNativeHanlder());
    auto  state  = glfwGetMouseButton(window, static_cast<int32_t>(button_code));
    return state == GLFW_PRESS;
}

bool vshade::event::Input::isGamepadButtonPressed(GamepadButtonCode const button_code, int const gamepad_id)
{
    GLFWgamepadstate state;
    if (glfwGetGamepadState(gamepad_id, &state))
    {
        return state.buttons[button_code] == GLFW_PRESS || state.buttons[button_code] == GLFW_REPEAT;
    }
    return false;
}

float vshade::event::Input::getGamepadAxies(GamepadAxies const axies, int const gamepad_id)
{
    int          axis_count{0};
    float const* axes = glfwGetJoystickAxes(gamepad_id, &axis_count);

    if (axes && axies >= 0 && axies < axis_count)
    {
        return axes[axies];
    }

    return 0.0f;
}

glm::ivec2 vshade::event::Input::getMousePosition()
{
    return window::Window::instance().getCursorPosition();
}

glm::ivec2 vshade::event::Input::getAbsoluteMousePosition()
{
    glm::ivec2 const cusros_position{getMousePosition()};
    glm::ivec2 const window_position{window::Window::instance().getWindowPosition()};
    return cusros_position + window_position;
}

void vshade::event::Input::setMousePosition(std::uint32_t const x, std::uint32_t const y)
{
    auto* window = static_cast<GLFWwindow*>(window::Window::instance().getWindowNativeHanlder());
    glfwSetCursorPos(window, x, y);
}

void vshade::event::Input::showMouseCursor(bool const show)
{
    auto* window = static_cast<GLFWwindow*>(window::Window::instance().getWindowNativeHanlder());
    glfwSetInputMode(window, GLFW_CURSOR, show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}
