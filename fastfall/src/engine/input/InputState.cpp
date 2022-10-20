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

InputState::InputState(const std::vector<InputType>& listen_inputs) {
    for (auto& in : listen_inputs) {
        listen(in);
    }
}

void InputState::update(secs deltaTime)
{
    if (deltaTime > 0.0) {
        process_events();
        for (auto& [type, input] : input_states) {
            input.update(deltaTime);
        }
    }
}

bool InputState::push_event(SDL_Event e)
{
    bool caught = false;
    switch (e.type) {
        case SDL_KEYDOWN:
            if (e.key.repeat != 0) {
                // discard repeats
                break;
            }
            else if (auto input_type = InputConfig::get_type(e.key.keysym.sym);
                input_type && is_listening(*input_type))
            {
                events.push_back({*input_type, true});
                caught = true;
            }
            break;
        case SDL_KEYUP:
            if (auto input_type = InputConfig::get_type(e.key.keysym.sym);
                input_type && is_listening(*input_type))
            {
                events.push_back({*input_type, false});
                caught = true;
            }
            break;
        //case SDL_CONTROLLERBUTTONDOWN:  break;
        //case SDL_CONTROLLERBUTTONUP:    break;
        //case SDL_CONTROLLERAXISMOTION:  break;
    }
    return caught;
}

void InputState::process_events() {
    for (const auto& event : events) {
        auto& input = at(event.type);

        if (event.active)
            input.activate();
        else
            input.deactivate();

    }
    events.clear();
}

// ------------------------------------------------------

void InputState::listen(InputType input) {
    input_states.emplace(input, Input{input});
}

void InputState::unlisten(InputType input) {
    input_states.erase(input);
    std::erase_if(events, [input](const auto& event) { return event.type == input; } );
}

void InputState::listen_config(const std::vector<InputType>& listen_inputs) {
    events.clear();
    input_states.clear();
    for (auto& in : listen_inputs) {
        listen(in);
    }
}
