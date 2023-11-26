#pragma once

#include "fastfall/engine/input/Input_Def.hpp"

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/Vec2.hpp"

#include <cfloat>

namespace ff {

class InputHandle {
public:
    constexpr static uint8_t MAG_ZERO = 0x0;
    constexpr static uint8_t MAG_FULL = 0xFF;

    InputHandle(Input t);
    InputHandle(const InputHandle& t) = default;
    InputHandle(InputHandle&& t) = default;
    InputHandle& operator=(const InputHandle& t) = default;
    InputHandle& operator=(InputHandle&& t) = default;

	void update(secs deltaTime);

	void reset();
	void activate();
	void deactivate();

	bool is_pressed(secs bufferWindow) const;
	bool is_held() const;
	void confirm_press();
    bool if_confirm_press(secs bufferWindow);

	bool is_active()		const { return active; };
	bool is_confirmed()		const { return confirmed; };
	bool is_confirmable()	const { return !confirmed; };

	int num_activators()	const { return activeCounter; };
	secs get_last_pressed() const { return lastPressed; }

    Input type() const { return m_type; }

    void set_magnitude(uint8_t mag) { curr_magnitude = mag; }
    uint8_t magnitude() const { return curr_magnitude; }

    int velocity() const { return curr_velocity; }

private:
    Input m_type;
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