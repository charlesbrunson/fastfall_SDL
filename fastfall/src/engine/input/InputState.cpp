#include "fastfall/engine/input/InputState.hpp"

namespace ff {

InputState::InputState(InputType t) :
	type(t)
{
}

void InputState::update(secs deltaTime)
{
	if (active) {
		if (firstFrame && lastPressed > 0.0) {
			firstFrame = false;
		}
		lastPressed += deltaTime;
	}
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
		firstFrame = false;
		lastHoldDuration = lastPressed;
		lastPressed = 0.0;
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