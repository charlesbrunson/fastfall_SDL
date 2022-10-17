#include "fastfall/engine/InputState.hpp"

#include "fastfall/engine/InputConfig.hpp"

using namespace ff;

InputState::listen_config_t InputState::config_gameplay = {
    InputType::UP,
    InputType::LEFT,
    InputType::DOWN,
    InputType::RIGHT,
    InputType::JUMP,
    InputType::DASH,
    InputType::ATTACK,
};

void InputState::update(secs deltaTime) {

}

void InputState::pushEvent(const SDL_Event &e) {

}

void InputState::resetState() {

}

void InputState::resetState(InputType map_input) {

}

void InputState::resetState(SDL_Keycode sdl_input) {

}

bool InputState::is_pressed(InputType input, secs bufferWindow) const {

}

bool InputState::is_held(InputType input) const {

}

void InputState::confirm_press(InputType input) {

}

bool InputState::is_pressed(SDL_Keycode input, secs bufferWindow) const {

}

bool InputState::is_held(SDL_Keycode input) const {

}

void InputState::confirm_press(SDL_Keycode input) {

}

void InputState::start_listen(InputType map_input) {

}

void InputState::start_listen(SDL_Keycode sdl_input) {

}

void InputState::stop_listen(InputType map_input) {

}

void InputState::stop_listen(SDL_Keycode sdl_input) {

}

void InputState::stop_listen_all() {

}

void InputState::listen_set_config(const listen_config_t& input_config) {

}

void InputState::listen_set_config(listen_config_t&& input_config) {

}

const InputState::listen_config_t& InputState::listen_get_config() const {

}

Input* InputState::get_state(const map_or_sdl_input_t& input) {
    auto it = input_states.find(input);
    return it != input_states.end() ? &it->second : nullptr;
}

void InputState::process_events()
{
    SDL_Event e;
    while (!event_queue.empty()) {
        e = event_queue.front();
        event_queue.pop();

        switch (e.type) {
            case SDL_KEYDOWN:               process_key_down(e);    break;
            case SDL_KEYUP:                 process_key_up(e);      break;
            case SDL_CONTROLLERBUTTONDOWN:  process_button_down(e); break;
            case SDL_CONTROLLERBUTTONUP:    process_button_up(e);   break;
            case SDL_CONTROLLERAXISMOTION:  process_axis_move(e);   break;
        }
    }
}

void InputState::process_key_down(const SDL_Event& e)
{
    if (e.key.repeat != 0) {
        // disregard repeats
        return;
    }
    else  {
        auto scancode = e.key.keysym.sym;
        auto state_it = input_states.end();

        if (auto it = InputConfig::get_input_type(scancode))
        {
            state_it = input_states.find(*it);
            InputConfig::notify(scancode, InputConfig::EventState::Active);
        }
        else {
            state_it = input_states.find(scancode);
        }

        if (state_it != input_states.end()) {
            state_it->second.activate();
        }
    }
}

void InputState::process_key_up(const SDL_Event& e) {

    auto scancode = e.key.keysym.sym;
    auto state_it = input_states.end();

    if (auto it = InputConfig::get_input_type(scancode))
    {
        state_it = input_states.find(*it);
        InputConfig::notify(scancode, InputConfig::EventState::Inactive);
    }
    else {
        state_it = input_states.find(scancode);
    }

    if (state_it != input_states.end()) {
        state_it->second.deactivate();
    }
}

void InputState::process_button_down(const SDL_Event& e) {

}

void InputState::process_button_up(const SDL_Event& e) {

}

void InputState::process_axis_move(const SDL_Event& e) {
    constexpr short JOY_AXIS_MAP_THRESHOLD = 25000;

}