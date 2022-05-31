#include "fastfall/engine/input/InputState.hpp"

namespace ff {

InputState::InputState(InputType t) :
	type(t)
{
}

void InputState::update(secs deltaTime)
{
	if (active) {
		if (firstFrame && lastPressed > 0.0 && lastPressed != DBL_MAX) {
			firstFrame = false;
		}
	}

	if (lastPressed >= DBL_MAX - deltaTime) {
		lastPressed = DBL_MAX;
	}
	else {
		lastPressed += deltaTime;
	}
}

void InputState::reset() {
	activeCounter = 0;
	active = false;
	confirmed = true;
	firstFrame = false;
	lastHoldDuration = lastPressed;
	lastPressed = DBL_MAX;
}
void InputState::activate() {
	if (activeCounter == 0) {
		active = true;
		confirmed = false;
		firstFrame = true;
		lastPressed = 0.0;
	}
	activeCounter++;
}
void InputState::deactivate() {
	activeCounter--;
	if (activeCounter <= 0) {
		activeCounter = 0;
		active = false;
		firstFrame = false;
		lastHoldDuration = lastPressed;
	}
}


bool InputState::is_pressed(secs bufferWindow) const {
	return is_confirmable()
		&& ((lastPressed <= bufferWindow) || (firstFrame && bufferWindow == 0.0));
}

bool InputState::is_held() const {
	return active;
}
void InputState::confirm_press() {
	confirmed = true;
}

}