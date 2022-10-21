#pragma once

#include "fastfall/engine/input/Input_Def.hpp"

namespace ff {

	namespace InputConfig {

		enum class GamepadInputType {
			NONE,
			BUTTON,
			AXIS
		};

		class GamepadInput {
		public:

            static constexpr bool AXIS_DIR_L = false;
            static constexpr bool AXIS_DIR_R = true;
            static constexpr bool AXIS_DIR_U = false;
            static constexpr bool AXIS_DIR_D = true;

			GamepadInputType type;

			// button data
			Button button;

			// axis data
			JoystickAxis axis;
			bool positiveSide = true;
			//short axisPrevPos = 0;

            static constexpr GamepadInput makeAxis(JoystickAxis axis, bool positiveEnd) {
                GamepadInput r;
                r.type = GamepadInputType::AXIS;
                r.axis = axis;
                r.positiveSide = positiveEnd;
                return r;
            };
            static constexpr GamepadInput makeButton(Button button) {
                GamepadInput r;
                r.type = GamepadInputType::BUTTON;
                r.button = button;
                return r;
            };

			std::string toString() const;
			bool operator< (const GamepadInput& rhs) const;

		};
	}

}
