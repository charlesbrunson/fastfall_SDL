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
			GamepadInputType type;

			// button data
			Button button;

			// axis data
			JoystickAxis axis;
			bool positiveSide = true;
			//short axisPrevPos = 0;

			static GamepadInput makeAxis(JoystickAxis axis, bool positiveEnd = true);
			static GamepadInput makeButton(Button button);
			std::string toString() const;
			bool operator< (const GamepadInput& rhs) const;

		};
	}

}
