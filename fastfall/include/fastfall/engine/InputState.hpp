#pragma once

#include <SDL_events.h>
#include <SDL_keycode.h>

#include "fastfall/engine/time/time.hpp"
#include "fastfall/engine/input/Input.hpp"
#include "fastfall/engine/input/Input_Def.hpp"
#include "fastfall/engine/input/GamepadInput.hpp"

#include "fastfall/util/math.hpp"

#include <queue>

namespace ff {

class InputState {
public:
    InputState() = default;
    InputState(std::vector<InputType> listen_inputs);


    void update(secs deltaTime);
    bool push_event(SDL_Event e);

    //bool is_pressed(InputType input, secs bufferWindow = 0.0) const;
    //bool is_held(InputType input) const;
    //bool confirm_press(InputType input);

    Input& operator[](InputType in) { return input_states.at(in); }
    const Input& operator[](InputType in) const { return input_states.at(in); }

    Input& at(InputType in) { return input_states.at(in); }
    const Input& at(InputType in) const { return input_states.at(in); }

    void listen(InputType input);
    void unlisten(InputType input);
    void listen_config(std::vector<InputType> listen_inputs);

    bool is_listening(InputType in) const { return input_states.contains(in); }
    bool is_listening(std::optional<InputType> in) const { return in && input_states.contains(*in); }

    static std::vector<InputType> config_gameplay;
private:
    Input* get_state(InputType input);

    void process_events();
    //bool process_key_down(SDL_KeyboardEvent e);
    //bool process_key_up(SDL_KeyboardEvent e);

    /*
    bool process_joystick_button_down(Button jbutton);
    bool process_joystick_button_up(Button jbutton);
    bool process_mouse_button_down(MouseButton mbutton);
    bool process_mouse_button_up(MouseButton mbutton);
    bool process_axis_move(JoystickAxis axis);
    */

    struct InputEvent {
        InputType type;
        bool active;
    };

    std::map<InputType, Input> input_states;
    std::vector<InputEvent> events;
};


}