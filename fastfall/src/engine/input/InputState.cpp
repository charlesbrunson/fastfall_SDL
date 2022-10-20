#include "fastfall/engine/InputState.hpp"

#include "fastfall/engine/InputConfig.hpp"

using namespace ff;

std::vector<InputType> InputState::config_gameplay = {
    InputType::UP,
    InputType::LEFT,
    InputType::DOWN,
    InputType::RIGHT,
    InputType::JUMP,
    InputType::DASH,
    InputType::ATTACK,
};

void InputState::update(secs deltaTime)
{
    if (deltaTime > 0.0) {
        //processEvents();
        for (auto& [type, input] : input_states) {
            input.update(deltaTime);
        }
    }
}

bool InputState::push_event(SDL_Event e) {

    switch (e.type) {
        case SDL_KEYDOWN:
            return process_key_down(e.key);
        case SDL_KEYUP:
            return process_key_up(e.key);
        //case SDL_CONTROLLERBUTTONDOWN:  break;
        //case SDL_CONTROLLERBUTTONUP:    break;
        //case SDL_CONTROLLERAXISMOTION:  break;
    }
    return false;
}

bool InputState::is_pressed(InputType input, secs bufferWindow) const {
    if (auto it = input_states.find(input);
        it != input_states.find(input))
    {
        return it->second.is_pressed(bufferWindow);
    }
    return false;
}

bool InputState::is_held(InputType input) const {
    if (auto it = input_states.find(input);
            it != input_states.find(input))
    {
        return it->second.is_held();
    }
    return false;
}

bool InputState::confirm_press(InputType input) {
    auto it = input_states.find(input);
    if (it != input_states.find(input))
    {
        it->second.confirm_press();
    }
    return it != input_states.find(input);
}

Input* InputState::get_state(InputType input) {
    auto it = input_states.find(input);
    return it != input_states.end() ? &it->second : nullptr;
}

bool InputState::process_key_down(SDL_KeyboardEvent e)
{
    // discard repeats
    if (e.repeat == 0) {
        auto scancode = e.keysym.sym;
        auto state_it = input_states.end();

        if (auto it = InputConfig::get_input_type_key(scancode))
        {
            state_it = input_states.find(*it);
            InputConfig::notify(scancode, InputConfig::EventState::Active);
        }

        bool has_value = state_it != input_states.end();
        if (has_value) {
            state_it->second.activate();
        }
        return has_value;
    }
    return false;
}

bool InputState::process_key_up(SDL_KeyboardEvent e) {

    auto scancode = e.keysym.sym;
    auto state_it = input_states.end();

    if (auto it = InputConfig::get_input_type_key(scancode))
    {
        state_it = input_states.find(*it);
        InputConfig::notify(scancode, InputConfig::EventState::Inactive);
    }

    bool has_value = state_it != input_states.end();
    if (has_value) {
        state_it->second.deactivate();
    }
    return has_value;
}
