#pragma once

#include "fastfall/engine/input/Input_Def.hpp"

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/Vec2.hpp"

#include <cfloat>

namespace ff {

class Input {
public:
    constexpr static uint8_t MAG_ZERO = 0x0;
    constexpr static uint8_t MAG_FULL = 0xFF;

    Input(InputType t);
    Input(const Input& t) = default;
    Input(Input&& t) = default;
    Input& operator=(const Input& t) = default;
    Input& operator=(Input&& t) = default;

	void update(secs deltaTime);

	void reset();
	void activate();
	void deactivate();

	bool is_pressed(secs bufferWindow) const;
	bool is_held() const;
	void confirm_press();

	bool is_active()		const { return active; };
	bool is_confirmed()		const { return confirmed; };
	bool is_confirmable()	const { return !confirmed; };

	int num_activators()	const { return activeCounter; };
	secs get_last_pressed() const { return lastPressed; }

    InputType type() const { return m_type; }

    void set_magnitude(uint8_t mag) { curr_magnitude = mag; }
    uint8_t magnitude() const { return curr_magnitude; }

    int velocity() const { return curr_velocity; }

	// for axis inputs
	//short axis_prev_pos = 0;
    //bool axis_prev_in_range = false;

	// for mouse inputs
	//Vec2i mouse_press_pos;
	//Vec2i mouse_release_pos;

private:
    InputType m_type;
	int activeCounter = 0; // num of inputs activating this

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