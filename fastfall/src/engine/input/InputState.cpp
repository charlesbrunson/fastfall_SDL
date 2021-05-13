#include "fastfall/engine/input/InputState.hpp"

namespace ff {

InputState::InputState(InputType t) :
	type(t)
{

}

void InputState::reset() {
	activeCounter = 0;
	active = false;
	confirmed = false;
	firstFrame = false;
	lastHoldDuration = lastPressed;
	lastPressed = 0.0;
}
void InputState::activate() {
	if (activeCounter == 0) {
		active = true;
		confirmed = false;
		firstFrame = true;
	}
	activeCounter++;
}
void InputState::deactivate() {
	activeCounter--;
	if (activeCounter <= 0) {
		activeCounter = 0;
		active = false;
		confirmed = false;
		firstFrame = false;
		lastHoldDuration = lastPressed;
		lastPressed = 0.0;
	}
}

}