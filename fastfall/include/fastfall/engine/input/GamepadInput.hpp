#pragma once

#include "fastfall/engine/input/Input_Def.hpp"

namespace ff {

	namespace Input {

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

			static GamepadInput Axis(JoystickAxis axis, bool positiveEnd = true);
			static GamepadInput Button(Button button);
			std::string toString() const;
			bool operator< (const GamepadInput& rhs) const;

		};
	}

}