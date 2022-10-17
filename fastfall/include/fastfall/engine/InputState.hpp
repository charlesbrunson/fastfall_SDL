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
    using map_or_sdl_input_t = std::variant<InputType, SDL_Keycode>;
    using listen_config_t = std::vector<map_or_sdl_input_t>;

    void update(secs deltaTime);
    void pushEvent(const SDL_Event &e);

    void resetState();
    void resetState(InputType map_input);
    void resetState(SDL_Keycode sdl_input);

    bool is_pressed(InputType input, secs bufferWindow = 0.0) const;
    bool is_held(InputType input) const;
    void confirm_press(InputType input);

    bool is_pressed(SDL_Keycode input, secs bufferWindow = 0.0) const;
    bool is_held(SDL_Keycode input) const;
    void confirm_press(SDL_Keycode input);

    void start_listen(InputType map_input);
    void start_listen(SDL_Keycode sdl_input);

    void stop_listen(InputType map_input);
    void stop_listen(SDL_Keycode sdl_input);
    void stop_listen_all();

    void listen_set_config(const listen_config_t& input_config);
    void listen_set_config(listen_config_t&& input_config);
    const listen_config_t& listen_get_config() const;

    static InputState::listen_config_t config_gameplay;
private:
    Input* get_state(const map_or_sdl_input_t& input);

    void process_events();

    void process_key_down(const SDL_Event& e);
    void process_key_up(const SDL_Event& e);

    void process_button_down(const SDL_Event& e);
    void process_button_up(const SDL_Event& e);

    void process_axis_move(const SDL_Event& e);

    listen_config_t config;
    std::unordered_map<map_or_sdl_input_t, Input> input_states;
    std::queue<SDL_Event> event_queue;
};


}