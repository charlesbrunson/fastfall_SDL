#pragma once

//#include "SDL_events.h"
//#include "SDL_keycode.h"

#include "fastfall/engine/time/time.hpp"
#include "fastfall/engine/input/Input.hpp"
#include "fastfall/engine/input/InputSource.hpp"
#include "fastfall/engine/input/Input_Def.hpp"
#include "fastfall/engine/input/GamepadInput.hpp"

#include "fastfall/util/math.hpp"

#include <queue>

namespace ff {

class InputState {
public:
    InputState();
    InputState(InputSource* source);

    InputState(const InputState& st);
    InputState(InputState&& st) noexcept;

    InputState& operator=(const InputState& st) = default;
    InputState& operator=(InputState&& st) noexcept = default;

    ~InputState();

    void set_source(InputSource* source);
    inline void reset_source() { set_source(nullptr); }

    InputSource* get_source() const { return input_source; }

    void update(secs deltaTime);

    Input& operator[](InputType in) { return input_states.at(in); }
    const Input& operator[](InputType in) const { return input_states.at(in); }

    Input& at(InputType in) { return input_states.at(in); }
    const Input& at(InputType in) const { return input_states.at(in); }

    Input* get(InputType in) { return input_states.contains(in) ? &at(in) : nullptr; }
    const Input* get(InputType in) const { return input_states.contains(in) ? &at(in) : nullptr; }

    bool is_listening(InputType in) const;
    bool is_listening(std::optional<InputType> in) const;

    void notify_unbind(InputType in);

    //void set_tick(size_t tick);
    size_t get_tick() const;

    const std::map<InputType, Input> all_inputs() const { return input_states; }

private:
    Input* get_state(InputType input);

    void process_events();
    void process_axis(const InputConfig::GamepadInput* gamepad, Input* input, int16_t axis_pos, int16_t alt_axis_pos);

    InputSource* input_source = nullptr;
    std::map<InputType, Input> input_states;
    size_t input_tick = 0;
};


}