#ifndef ENGINE_CORE_EVENTS_KEYCODES_H
#define ENGINE_CORE_EVENTS_KEYCODES_H

namespace vshade
{
namespace event
{

using KeyCode           = unsigned short;
using MouseButtonCode   = unsigned short;
using GamepadButtonCode = unsigned short;
using GamepadAxies      = unsigned short;

namespace mouse_button
{
enum : MouseButtonCode // From glfw3.h
{
    _0_      = 0,
    _1_      = 1,
    _2_      = 2,
    _3_      = 3,
    _4_      = 4,
    _5_      = 5,
    _6_      = 6,
    _7_      = 7,
    _LAST_   = _7_,
    _LEFT_   = _0_,
    _RIGTH_  = _1_,
    _MIDDLE_ = _2_
};
}
namespace gamepad_button
{
enum GamepadButtonCode : int
{
    _GAMEPAD_BUTTON_A_            = 0,
    _GAMEPAD_BUTTON_B_            = 1,
    _GAMEPAD_BUTTON_X_            = 2,
    _GAMEPAD_BUTTON_Y_            = 3,
    _GAMEPAD_BUTTON_LEFT_BUMPER_  = 4,
    _GAMEPAD_BUTTON_RIGHT_BUMPER_ = 5,
    _GAMEPAD_BUTTON_BACK_         = 6,
    _GAMEPAD_BUTTON_START_        = 7,
    _GAMEPAD_BUTTON_GUIDE_        = 8,
    _GAMEPAD_BUTTON_LEFT_THUMB_   = 9,
    _GAMEPAD_BUTTON_RIGHT_THUMB_  = 10,
    _GAMEPAD_BUTTON_DPAD_UP_      = 11,
    _GAMEPAD_BUTTON_DPAD_RIGHT_   = 12,
    _GAMEPAD_BUTTON_DPAD_DOWN_    = 13,
    _GAMEPAD_BUTTON_DPAD_LEFT_    = 14
};
} // namespace gamepad_button

namespace gamepad_axis
{
enum GamepadAxies : int
{
    _GAMEPAD_AXIS_LEFT_X_        = 0,
    _GAMEPAD_AXIS_LEFT_Y_        = 1,
    _GAMEPAD_AXIS_RIGHT_X_       = 2,
    _GAMEPAD_AXIS_RIGHT_Y_       = 3,
    _GAMEPAD_AXIS_LEFT_TRIGGER_  = 4,
    _GAMEPAD_AXIS_RIGHT_TRIGGER_ = 5,
    _GAMEPAD_AXIS_RIGHT_LAST_    = _GAMEPAD_AXIS_RIGHT_TRIGGER_
};
} // namespace gamepad_axis

namespace key
{
enum : KeyCode // From glfw3.h
{

    _KEY_SPACE_      = 32,
    _KEY_APOSTROPHE_ = 39,
    _KEY_COMMA_      = 44,
    _KEY_MINUS_      = 45,
    _KEY_PERIOD_     = 46,
    _KEY_SLASH_      = 47,

    _KEY_D0_ = 48,
    _KEY_D1_ = 49,
    _KEY_D2_ = 50,
    _KEY_D3_ = 51,
    _KEY_D4_ = 52,
    _KEY_D5_ = 53,
    _KEY_D6_ = 54,
    _KEY_D7_ = 55,
    _KEY_D8_ = 56,
    _KEY_D9_ = 57,

    _KEY_SEMICOLON_ = 59,
    _KEY_EQUAL_     = 61,

    _KEY_A_ = 65,
    _KEY_B_ = 66,
    _KEY_C_ = 67,
    _KEY_D_ = 68,
    _KEY_E_ = 69,
    _KEY_F_ = 70,
    _KEY_G_ = 71,
    _KEY_H_ = 72,
    _KEY_I_ = 73,
    _KEY_J_ = 74,
    _KEY_K_ = 75,
    _KEY_L_ = 76,
    _KEY_M_ = 77,
    _KEY_N_ = 78,
    _KEY_O_ = 79,
    _KEY_P_ = 80,
    _KEY_Q_ = 81,
    _KEY_R_ = 82,
    _KEY_S_ = 83,
    _KEY_T_ = 84,
    _KEY_U_ = 85,
    _KEY_V_ = 86,
    _KEY_W_ = 87,
    _KEY_X_ = 88,
    _KEY_Y_ = 89,
    _KEY_Z_ = 90,

    _KEY_LEFTBRACKET_  = 91,
    _KEY_BACKSLASH_    = 92,
    _KEY_RIGHTBRACKET_ = 93,
    _KEY_GRAVEACCENT_  = 96,

    _KEY_WORLD1_ = 161,
    _KEY_WORLD2_ = 162,

    _KEY_ESCAPE_      = 256,
    _KEY_ENTER_       = 257,
    _KEY_TAB_         = 258,
    _KEY_BACKSPACE_   = 259,
    _KEY_INSERT_      = 260,
    _KEY_DELETE_      = 261,
    _KEY_RIGHT_       = 262,
    _KEY_LEFT_        = 263,
    _KEY_DOWN_        = 264,
    _KEY_UP_          = 265,
    _KEY_PAGEUP_      = 266,
    _KEY_PAGEDOWN_    = 267,
    _KEY_HOME_        = 268,
    _KEY_END_         = 269,
    _KEY_CAPSLOCK_    = 280,
    _KEY_SCROLLLOCK_  = 281,
    _KEY_NUMLOCK_     = 282,
    _KEY_PRINTSCREEN_ = 283,
    _KEY_PAUSE_       = 284,
    _KEY_F1_          = 290,
    _KEY_F2_          = 291,
    _KEY_F3_          = 292,
    _KEY_F4_          = 293,
    _KEY_F5_          = 294,
    _KEY_F6_          = 295,
    _KEY_F7_          = 296,
    _KEY_F8_          = 297,
    _KEY_F9_          = 298,
    _KEY_F10_         = 299,
    _KEY_F11_         = 300,
    _KEY_F12_         = 301,
    _KEY_F13_         = 302,
    _KEY_F14_         = 303,
    _KEY_F15_         = 304,
    _KEY_F16_         = 305,
    _KEY_F17_         = 306,
    _KEY_F18_         = 307,
    _KEY_F19_         = 308,
    _KEY_F20_         = 309,
    _KEY_F21_         = 310,
    _KEY_F22_         = 311,
    _KEY_F23_         = 312,
    _KEY_F24_         = 313,
    _KEY_F25_         = 314,

    _KEY_KP0_        = 320,
    _KEY_KP1_        = 321,
    _KEY_KP2_        = 322,
    _KEY_KP3_        = 323,
    _KEY_KP4_        = 324,
    _KEY_KP5_        = 325,
    _KEY_KP6_        = 326,
    _KEY_KP7_        = 327,
    _KEY_KP8_        = 328,
    _KEY_KP9_        = 329,
    _KEY_KPDECIMAL_  = 330,
    _KEY_KPDIVIDE_   = 331,
    _KEY_KPMULTIPLY_ = 332,
    _KEY_KPSUBTRACT_ = 333,
    _KEY_KPADD_      = 334,
    _KEY_KPENTER_    = 335,
    _KEY_KPEQUAL_    = 336,

    _KEY_LEFTSHIFT_    = 340,
    _KEY_LEFTCONTROL_  = 341,
    _KEY_LEFTALT_      = 342,
    _KEY_LEFTSUPER_    = 343,
    _KEY_RIGHTSHIFT_   = 344,
    _KEY_RIGHTCONTROL_ = 345,
    _KEY_RIGHTALT_     = 346,
    _KEY_RIGHTSUPER_   = 347,
    _KEY_MENU_         = 348
};
}
} // namespace event

} // namespace vshade

#endif // ENGINE_CORE_EVENTS_KEYCODES_H
