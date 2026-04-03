#ifndef ENGINE_CORE_EVENTS_INPUT_H
#define ENGINE_CORE_EVENTS_INPUT_H

#include <cstdint>
#include <engine/config/vshade_api.h>
#include <engine/core/events/key_codes.h>
#include <glm/glm/glm.hpp>
#include <utility>
namespace vshade
{
namespace event
{
class VSHADE_API Input
{
public:
    static bool       isKeyPressed(KeyCode const key_code);
    static bool       isMouseButtonPressed(MouseButtonCode const button_code);
    static bool       isGamepadButtonPressed(GamepadButtonCode const button_code, int const gamepad_id = 0);
    static float      getGamepadAxies(GamepadAxies const axies, int const gamepad_id = 0);
    static glm::ivec2 getMousePosition();
    static glm::ivec2 getAbsoluteMousePosition();
    static void       setMousePosition(std::uint32_t const x, std::uint32_t const y);
    static void       showMouseCursor(bool const show);
};
} // namespace event
} // namespace vshade

#endif // ENGINE_CORE_EVENTS_INPUT_H