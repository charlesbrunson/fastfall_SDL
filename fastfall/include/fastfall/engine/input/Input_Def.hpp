#pragma once

#include <map>
#include <string>

namespace ff {

enum class InputType : int {
	NONE	= -1,
	UP		= 0,
	LEFT	= 1,
	DOWN,
	RIGHT,
	JUMP,
	DASH,
	ATTACK,
	//MOUSE1,
	//MOUSE2,
	COUNT
};
static constexpr unsigned int INPUT_COUNT = static_cast<unsigned>(InputType::COUNT);

using Button        = unsigned int;
using JoystickAxis  = uint8_t;
using MouseButton   = uint8_t;

}