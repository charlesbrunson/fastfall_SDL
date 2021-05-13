#pragma once

#include <map>
#include <string>

namespace ff {

enum class InputMethod {
	KEYBOARD,
	MOUSE,
	JOYSTICK
};

enum InputType : int {
	NONE = -1,
	UP = 0,
	LEFT = 1,
	DOWN,
	RIGHT,
	JUMP,
	DASH,
	ATTACK,
	MOUSE1,
	MOUSE2,
	COUNT
};
static constexpr unsigned int INPUT_COUNT = InputType::COUNT;

namespace Input {
	using Button = unsigned int;
	using JoystickAxis = uint8_t;
	using MouseButton = uint8_t;
}

}