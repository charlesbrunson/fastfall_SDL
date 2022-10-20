#pragma once

#include <SDL_events.h>
#include <SDL_keycode.h>

#include "fastfall/engine/time/time.hpp"
#include "fastfall/engine/input/Input.hpp"
#include "fastfall/engine/input/Input_Def.hpp"
#include "fastfall/engine/input/GamepadInput.hpp"

#include "fastfall/util/math.hpp"

#include <variant>
#include <queue>

namespace ff {

class InputState {
public:
    InputState() = default;
    InputState(std::vector<InputType> listen_inputs);


    void update(secs deltaTime);
    bool push_event(SDL_Event e);

    bool is_pressed(InputType input, secs bufferWindow = 0.0) const;
    bool is_held(InputType input) const;
    bool confirm_press(InputType input);

    void listen(InputType input);
    void unlisten(InputType input);

    void listen_config(std::vector<InputType> listen_inputs);


    static std::vector<InputType> config_gameplay;
private:
    Input* get_state(InputType input);

    //void process_events();
    bool process_key_down(SDL_KeyboardEvent e);
    bool process_key_up(SDL_KeyboardEvent e);

    /*
    bool process_joystick_button_down(Button jbutton);
    bool process_joystick_button_up(Button jbutton);
    bool process_mouse_button_down(MouseButton mbutton);
    bool process_mouse_button_up(MouseButton mbutton);
    bool process_axis_move(JoystickAxis axis);
    */

    std::unordered_map<InputType, Input> input_states;
};


}