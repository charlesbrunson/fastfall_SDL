#pragma once

#include "fastfall/engine/input/Input_Def.hpp"

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/Vec2.hpp"

namespace ff {

class InputState {
public:
	InputState(InputType t);

	void reset();
	void activate();
	void deactivate();

	InputType type;

	int activeCounter = 0; // num of inputs activating this

	bool enabled = true;
	bool active = false;
	bool confirmed = false;
	bool firstFrame = false;

	short axisPrevPos = 0;

	secs lastPressed = 0.0;
	secs lastHoldDuration = 0.0;

	// for mouse inputs
	Vec2i lastPressPosition;
	Vec2i lastReleasePosition;

};

}