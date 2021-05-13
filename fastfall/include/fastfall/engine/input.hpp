#pragma once

#include <SDL_events.h>
#include <SDL_keycode.h>

#include "fastfall/engine/time/time.hpp"
#include "fastfall/engine/imgui/ImGuiContent.hpp"
#include "fastfall/engine/input/Input_Def.hpp"
#include "fastfall/engine/input/GamepadInput.hpp"

#include "fastfall/util/math.hpp"

#include <set>
#include <map>

namespace ff {

namespace Input {

	class InputObserver : public ImGuiContent {
	public:
		InputObserver();
		void ImGui_getContent();
	};

	struct InputHold {
		bool isReleased = false;
		secs holdDuration = 0.0;
	};

	void setAxisDeadzone(float deadzone = 0.1f);
	float getAxisDeadzone();

	void update(secs deltaTime);
	void pushEvent(const SDL_Event& e);

	void bindInput(InputType input, SDL_Keycode key);
	void bindInput(InputType input, GamepadInput gamepad);

	void unbind(InputType input);

	void resetState();

	bool isPressed(InputType input, secs bufferWindow = 0.0);
	bool isHeld(InputType input);

	void confirmPress(InputType input);

	Vec2i getMouseWindowPosition();

	bool mousePositionUpdated();
	Vec2f getMouseWorldPosition();
	void setMouseWorldPosition(Vec2f pos);

	void setMouseInView(bool in_view);
	bool getMouseInView();

	void updateJoystick();
	void closeJoystick();

	extern bool debugEvents;
}

}