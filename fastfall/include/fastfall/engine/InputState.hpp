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
    InputState(const std::vector<InputType>& listen_inputs);

    void update(secs deltaTime);
    bool push_event(SDL_Event e);

    Input& operator[](InputType in) { return input_states.at(in); }
    const Input& operator[](InputType in) const { return input_states.at(in); }

    Input& at(InputType in) { return input_states.at(in); }
    const Input& at(InputType in) const { return input_states.at(in); }

    Input* get(InputType in) { return input_states.contains(in) ? &at(in) : nullptr; }
    const Input* get(InputType in) const { return input_states.contains(in) ? &at(in) : nullptr; }

    bool is_listening(InputType in) const;
    bool is_listening(std::optional<InputType> in) const;

    void set_tick(size_t tick);
    size_t get_tick() const;

    static std::vector<InputType> config_gameplay;
private:
    Input* get_state(InputType input);

    void process_events();
    void process_axis(const InputConfig::GamepadInput* gamepad, Input* input, int16_t axis_pos, int16_t alt_axis_pos);

    struct InputEvent {
        InputType type;
        uint8_t magnitude;
        //bool active;
    };

    std::map<InputType, Input> input_states;
    std::vector<InputEvent> events;
    size_t input_tick = 0;
};


}