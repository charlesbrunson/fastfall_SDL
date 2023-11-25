#pragma once

//#include "SDL_events.h"
//#include "SDL_keycode.h"

#include "fastfall/engine/time/time.hpp"
#include "fastfall/engine/input/InputHandle.hpp"
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

    InputHandle& operator[](Input in) { return input_states.at(in); }
    const InputHandle& operator[](Input in) const { return input_states.at(in); }

    InputHandle& at(Input in) { return input_states.at(in); }
    const InputHandle& at(Input in) const { return input_states.at(in); }

    InputHandle* get(Input in) { return input_states.contains(in) ? &at(in) : nullptr; }
    const InputHandle* get(Input in) const { return input_states.contains(in) ? &at(in) : nullptr; }

    bool is_listening(Input in) const;
    bool is_listening(std::optional<Input> in) const;

    void notify_unbind(Input in);

    //void set_tick(size_t tick);
    size_t get_tick() const;

    const std::map<Input, InputHandle> all_inputs() const { return input_states; }

private:
    InputHandle* get_state(Input input);

    void process_events();
    void process_axis(const InputConfig::GamepadInput* gamepad, InputHandle* input, int16_t axis_pos, int16_t alt_axis_pos);

    InputSource* input_source = nullptr;
    std::map<Input, InputHandle> input_states;
    size_t input_tick = 0;
};


}