#include "fastfall/engine/input/InputHandle.hpp"

namespace ff {

InputHandle::InputHandle(Input t) :
	m_type(t)
{
}

void InputHandle::update(secs deltaTime)
{
    if (firstFrame && lastPressed > 0.0) {
        firstFrame = false;
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

void InputHandle::reset() {
	activeCounter = 0;
	active = false;
	confirmed = true;
	firstFrame = false;
	lastHoldDuration = lastPressed;
	lastPressed = DBL_MAX;
}

void InputHandle::activate() {
	if (activeCounter == 0) {
		active = true;
		confirmed = false;
		firstFrame = true;
		lastPressed = 0.0;
	}
	activeCounter++;
}

void InputHandle::deactivate() {
	activeCounter--;
	if (activeCounter <= 0) {
		activeCounter = 0;
		active = false;
		//firstFrame = false;
		lastHoldDuration = lastPressed;
	}
}

bool InputHandle::is_pressed(secs bufferWindow) const {
	return is_confirmable()
		&& ((lastPressed <= bufferWindow) || (firstFrame && bufferWindow == 0.0));
}

bool InputHandle::is_held() const {
	return active;
}

void InputHandle::confirm_press() {
	confirmed = true;
}

}