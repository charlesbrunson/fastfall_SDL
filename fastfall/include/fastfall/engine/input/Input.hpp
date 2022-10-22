#pragma once

#include "fastfall/engine/input/Input_Def.hpp"

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/Vec2.hpp"

#include <cfloat>

namespace ff {

class Input {
public:
    Input(InputType t);

	void update(secs deltaTime);

	void reset();
	void activate();
	void deactivate();

	bool is_pressed(secs bufferWindow) const;
	bool is_held() const;
	void confirm_press();

	//bool is_enabled()		const { return enabled; };
	bool is_active()		const { return active; };
	bool is_confirmed()		const { return confirmed; };
	bool is_confirmable()	const { return /* active && */ !confirmed; };

	int num_activators()	const { return activeCounter; };
	secs get_last_pressed() const { return lastPressed; }

	const InputType type;

	// for axis inputs
	//short axis_prev_pos = 0;
    bool axis_prev_in_range = false;

	// for mouse inputs
	Vec2i mouse_press_pos;
	Vec2i mouse_release_pos;

    constexpr static uint8_t MAG_ZERO = 0x0;
    constexpr static uint8_t MAG_FULL = 0xFF;

    void set_magnitude(uint8_t mag) { curr_magnitude = mag; }

    uint8_t magnitude() { return curr_magnitude; }
    int velocity() { return curr_velocity; }

    /*
    void update(secs deltaTime) {
        velocity = (int)(((double)magnitude - (double)prev_magnitude) / deltaTime);
        prev_magnitude = magnitude;
    }
    */

private:

	int activeCounter = 0; // num of inputs activating this

	//bool enabled = true;
	bool active = false;
	bool confirmed = true;
	bool firstFrame = false;

	secs lastPressed = DBL_MAX;
	secs lastHoldDuration = 0.0;

    uint8_t curr_magnitude = MAG_ZERO;
    uint8_t prev_magnitude = MAG_ZERO;
    int curr_velocity = 0;
};

}