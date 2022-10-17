#include "fastfall/engine/input/GamepadInput.hpp"

#include "fastfall/engine/input/Input_Def.hpp"

#include <iostream>
#include <sstream>


using namespace ff::InputConfig;

GamepadInput GamepadInput::makeAxis(JoystickAxis axis, bool positiveEnd) {
	GamepadInput r;
	r.type = GamepadInputType::AXIS;
	r.axis = axis;
	r.positiveSide = positiveEnd;
	return r;
};
GamepadInput GamepadInput::makeButton(Button button) {
	GamepadInput r;
	r.type = GamepadInputType::BUTTON;
	r.button = button;
	return r;
};

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
