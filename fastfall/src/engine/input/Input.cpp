#include "fastfall/engine/input/Input.hpp"

namespace ff {

Input::Input(InputType t) :
	type(t)
{
}

void Input::update(secs deltaTime)
{
	if (active) {
		if (firstFrame && lastPressed > 0.0) {
			firstFrame = false;
		}
	}

	if (lastPressed >= DBL_MAX - deltaTime) {
		lastPressed = DBL_MAX;
	}
	else {
		lastPressed += deltaTime;
	}

    curr_velocity = (int)(((double)curr_magnitude - (double)prev_magnitude) / deltaTime);
    prev_magnitude = curr_magnitude;
}

void Input::reset() {
	activeCounter = 0;
	active = false;
	confirmed = true;
	firstFrame = false;
	lastHoldDuration = lastPressed;
	lastPressed = DBL_MAX;
}

void Input::activate() {
	if (activeCounter == 0) {
		active = true;
		confirmed = false;
		firstFrame = true;
		lastPressed = 0.0;
	}
	activeCounter++;
}

void Input::deactivate() {
	activeCounter--;
	if (activeCounter <= 0) {
		activeCounter = 0;
		active = false;
		firstFrame = false;
		lastHoldDuration = lastPressed;
	}
}

bool Input::is_pressed(secs bufferWindow) const {
	return is_confirmable()
		&& ((lastPressed <= bufferWindow) || (firstFrame && bufferWindow == 0.0));
}

bool Input::is_held() const {
	return active;
}

void Input::confirm_press() {
	confirmed = true;
}

}