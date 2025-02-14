#pragma once

#include <map>
#include <string>

#include <cstdint>

namespace ff {

enum class Input : int {
	None	= -1,
	Up		= 0,
	Left	= 1,
	Down,
	Right,
	Jump,
	Dash,
	Attack,
	//MOUSE1,
	//MOUSE2,
	Count
};
static constexpr unsigned int INPUT_COUNT = static_cast<unsigned>(Input::Count);

using Button        = unsigned int;
using JoystickAxis  = uint8_t;
using MouseButton   = uint8_t;

}