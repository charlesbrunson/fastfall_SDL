#include "fastfall/engine/input/GamepadInput.hpp"

#include "fastfall/engine/input/Input_Def.hpp"

#include <iostream>
#include <sstream>


using namespace ff::InputConfig;


std::string GamepadInput::toString() const {
	std::stringstream ss;
	if (type == GamepadInputType::BUTTON) {
		ss << "Button ";
		ss << button;
	}
	else {
		ss << "Axis" << (positiveSide ? "+ " : "- ");
		ss << (int)axis;
	}
	return ss.str();
};

bool GamepadInput::operator< (const GamepadInput& rhs) const {
	if (type == rhs.type) {
		if (type == GamepadInputType::BUTTON) {
			return button < rhs.button;
		}
		else {
			if (axis == rhs.axis) {
				if (positiveSide && positiveSide != rhs.positiveSide) {
					return true;
				}
				return false;
			}
			else {
				return axis < rhs.axis;
			}
		}
	}
	else {
		return type < rhs.type;
	}
}
