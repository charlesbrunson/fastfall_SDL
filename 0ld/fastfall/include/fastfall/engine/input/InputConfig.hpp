#pragma once

#include "SDL_events.h"
#include "SDL_keycode.h"

#include "fastfall/engine/time/time.hpp"
#include "fastfall/engine/imgui/ImGuiContent.hpp"
#include "fastfall/engine/input/Input_Def.hpp"
#include "fastfall/engine/input/GamepadInput.hpp"

#include "fastfall/util/math.hpp"
#include "fastfall/engine/input/InputState.hpp"

#include <set>
#include <map>

namespace ff::InputConfig {

	class InputObserver : public ImGuiContent {
	public:
		InputObserver();
		void ImGui_getContent(secs deltaTime);
	};

	void setAxisDeadzone(short deadzone);
	short getAxisDeadzone();

    std::optional<Input> is_waiting_for_bind();
	void bindInput(Input input, SDL_Keycode key);
	void bindInput(Input input, GamepadInput gamepad);

	void unbind(Input input);

    std::optional<Input> get_type_key(SDL_Keycode code);
    std::optional<Input> get_type_jbutton(Button jbutton);

    std::pair<std::optional<Input>, std::optional<Input>>
    get_type_jaxis(JoystickAxis axis);

    void add_listener(InputState& listen);
    void remove_listener(InputState& listen);

    enum class EventState {
        Active,
        Inactive
    };

	void updateJoystick();
	void closeJoystick();

    bool configExists();
    bool writeConfigFile();
    bool readConfigFile();
}
