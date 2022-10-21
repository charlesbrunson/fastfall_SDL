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
        case SDL_KEYDOWN: {
            if (e.key.repeat != 0) {
                // discard repeats
                break;
            } else if (auto input_type = InputConfig::get_type_key(e.key.keysym.sym);
                input_type && is_listening(*input_type))
            {
                events.push_back({*input_type, true});
                caught = true;
            }
        }
        break;
        case SDL_KEYUP: {
            if (auto input_type = InputConfig::get_type_key(e.key.keysym.sym);
                input_type && is_listening(*input_type))
            {
                events.push_back({*input_type, false});
                caught = true;
            }
        }
        break;
        case SDL_CONTROLLERBUTTONDOWN: {
            if (auto input_type = InputConfig::get_type_jbutton(e.cbutton.button);
                input_type && is_listening(*input_type))
            {
                events.push_back({*input_type, true});
                caught = true;
            }
        }
        break;
        case SDL_CONTROLLERBUTTONUP: {
            if (auto input_type = InputConfig::get_type_jbutton(e.cbutton.button);
                input_type && is_listening(*input_type))
            {
                events.push_back({*input_type, false});
                caught = true;
            }
        }
        break;
        case SDL_CONTROLLERAXISMOTION: {

            // TODO implement circular deadzone, currently square

            auto [axis1, axis2] = InputConfig::get_type_jaxis(e.caxis.axis);
            int16_t axis_position = e.caxis.value;

            auto gamepad1 = InputConfig::getGamepadInput(e.caxis.axis, false);
            auto gamepad2 = InputConfig::getGamepadInput(e.caxis.axis, true);

            Input* input1 = axis1 ? get(*axis1) : nullptr;
            Input* input2 = axis2 ? get(*axis2) : nullptr;

            constexpr auto inRange = [](bool side, float position) {
                return side
                    ? position >= InputConfig::getAxisDeadzone()
                    : position <= -InputConfig::getAxisDeadzone();
            };

            auto checkAxisSide = [&](const InputConfig::GamepadInput* gamepad, Input* input) {
                if (gamepad && input) {
                    bool inrangeCur = inRange(gamepad->positiveSide, axis_position);
                    bool inrangePrev = inRange(gamepad->positiveSide, input->axis_prev_pos);

                    if (inrangeCur != inrangePrev) {
                        inrangeCur ? input->activate() : input->deactivate();
                    }
                    input->axis_prev_pos = e.caxis.value;
                }
            };

            checkAxisSide(gamepad1, input1);
            checkAxisSide(gamepad2, input2);
        }
        break;
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
