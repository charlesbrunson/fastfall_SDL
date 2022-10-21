#include "fastfall/engine/InputState.hpp"

#include "fastfall/engine/InputConfig.hpp"
#include "fastfall/util/log.hpp"

using namespace ff;

void InputState::process_axis(const InputConfig::GamepadInput* gamepad, Input* input, int16_t axis_pos, int16_t alt_axis_pos) {
    if (gamepad && input) {
        bool inrangeCur = false;

        constexpr int16_t max = std::numeric_limits<int16_t>::max();
        int16_t deadzone = InputConfig::getAxisDeadzone();
        size_t deadzone_squared = (size_t)deadzone * (size_t)deadzone;
        size_t axis_squared = (size_t)axis_pos * (size_t)axis_pos + (size_t)alt_axis_pos * (size_t)alt_axis_pos;
        uint8_t magnitude = 0;

        if ((gamepad->positiveSide && axis_pos > 0)
            || (!gamepad->positiveSide && axis_pos < 0))
        {
            if (abs(axis_pos) > abs(alt_axis_pos) / 2) {
                if (axis_squared > deadzone_squared) {
                    inrangeCur = true;
                    //double dbl_mag = sqrt((double)axis_squared) - (double)deadzone;
                    //LOG_INFO("mag:{}\t{}\t{}", dbl_mag, sqrt((double)axis_squared), deadzone);
                    //magnitude = (uint8_t)((dbl_mag / (max - (double)deadzone)) * 255.0);
                    magnitude = ((size_t)abs(axis_pos) - (size_t)deadzone) * (size_t)0xFF / ((size_t)max - (size_t)deadzone);
                }
            }
        }

        LOG_INFO("axis:{} mag:{}", SDL_GameControllerGetStringForAxis((SDL_GameControllerAxis)gamepad->axis), magnitude);

        if (inrangeCur != input->axis_prev_in_range) {
            events.push_back({input->type, magnitude});
        }
        input->axis_prev_in_range = inrangeCur;
    }
}

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
                events.push_back({*input_type, Input::MAG_FULL});
                caught = true;
            }
        }
        break;
        case SDL_KEYUP: {
            if (auto input_type = InputConfig::get_type_key(e.key.keysym.sym);
                input_type && is_listening(*input_type))
            {
                events.push_back({*input_type, Input::MAG_ZERO});
                caught = true;
            }
        }
        break;
        case SDL_CONTROLLERBUTTONDOWN: {
            if (auto input_type = InputConfig::get_type_jbutton(e.cbutton.button);
                input_type && is_listening(*input_type))
            {
                events.push_back({*input_type, Input::MAG_FULL});
                caught = true;
            }
        }
        break;
        case SDL_CONTROLLERBUTTONUP: {
            if (auto input_type = InputConfig::get_type_jbutton(e.cbutton.button);
                input_type && is_listening(*input_type))
            {
                events.push_back({*input_type, Input::MAG_ZERO});
                caught = true;
            }
        }
        break;
        case SDL_CONTROLLERAXISMOTION: {
            // TODO implement circular deadzone, currently square

            JoystickAxis axis = e.caxis.axis;
            int16_t axis_pos = e.caxis.value;

            std::optional<JoystickAxis> alt_axis;
            int16_t alt_axis_pos = 0;

            switch (e.caxis.axis) {
                case SDL_CONTROLLER_AXIS_LEFTX:
                    alt_axis = SDL_CONTROLLER_AXIS_LEFTY;
                    break;
                case SDL_CONTROLLER_AXIS_LEFTY:
                    alt_axis = SDL_CONTROLLER_AXIS_LEFTX;
                    break;
                case SDL_CONTROLLER_AXIS_RIGHTX:
                    alt_axis = SDL_CONTROLLER_AXIS_RIGHTY;
                    break;
                case SDL_CONTROLLER_AXIS_RIGHTY:
                    alt_axis = SDL_CONTROLLER_AXIS_RIGHTX;
                    break;
            }
            if (alt_axis) {
                alt_axis_pos = SDL_JoystickGetAxis(SDL_JoystickFromInstanceID(e.caxis.which), *alt_axis);
            }

            auto [axis1, axis2] = InputConfig::get_type_jaxis(e.caxis.axis);
            process_axis(InputConfig::getGamepadInput(e.caxis.axis, false), (axis1 ? get(*axis1) : nullptr), axis_pos, alt_axis_pos);
            process_axis(InputConfig::getGamepadInput(e.caxis.axis, true), (axis2 ? get(*axis2) : nullptr), axis_pos, alt_axis_pos);

            if (alt_axis) {
                auto [alt1, alt2] = InputConfig::get_type_jaxis(*alt_axis);
                process_axis(InputConfig::getGamepadInput(*alt_axis, false), (alt1 ? get(*alt1) : nullptr), alt_axis_pos, axis_pos);
                process_axis(InputConfig::getGamepadInput(*alt_axis, true), (alt2 ? get(*alt2) : nullptr), alt_axis_pos, axis_pos);

                /*
                if (axis < *alt_axis) {
                    LOG_INFO("a1:{} a2:{}", axis_pos, alt_axis_pos);
                }
                else {
                    LOG_INFO("a1:{} a2:{}", alt_axis_pos, axis_pos);
                }
                */
            }
        }
        break;
    }
    return caught;
}

void InputState::process_events() {
    for (const auto& event : events) {
        auto& input = at(event.type);

        if (event.magnitude > 0) {
            input.activate();
        }
        else {
            input.deactivate();
        }
        input.magnitude = event.magnitude;
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
