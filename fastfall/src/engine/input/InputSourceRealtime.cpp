#include "fastfall/engine/input/InputSourceRealtime.hpp"

#include "fastfall/engine/input/InputHandle.hpp"
#include "fastfall/engine/input/InputConfig.hpp"
#include "fastfall/util/log.hpp"

#include <assert.h>

using namespace ff;

void InputSourceRealtime::process_axis(Input type, AxisData& data, int16_t axis_pos, int16_t alt_axis_pos) {
    bool curr_in_range = false;

    constexpr int16_t max = std::numeric_limits<int16_t>::max();
    int16_t deadzone = InputConfig::getAxisDeadzone();
    size_t deadzone_squared = (size_t)deadzone * (size_t)deadzone * 2;
    size_t axis_squared = (size_t)axis_pos * (size_t)axis_pos + (size_t)alt_axis_pos * (size_t)alt_axis_pos;
    uint8_t magnitude = 0;

    if ((data.positive_side && axis_pos > 0)
        || (!data.positive_side && axis_pos < 0))
    {
        if (abs(axis_pos) >= abs(alt_axis_pos) / 2) {
            if (axis_squared >= deadzone_squared) {
                curr_in_range = true;
                magnitude = 1 + ((size_t)abs(axis_pos) - (size_t)deadzone) * (size_t)0xFE / ((size_t)max - (size_t)deadzone);
            }
        }
    }

    events.push_back({type, (curr_in_range != data.prev_in_range), magnitude});
    data.prev_in_range = curr_in_range;
}

InputSourceRealtime::InputSourceRealtime(const std::set<Input>& accept_inputs, secs deltaTime, RecordInputs record_inputs)
    : InputSource(accept_inputs)
{
    assert(deltaTime > 0.0);
    listening = accept_inputs;
    if (record_inputs == RecordInputs::Yes)
    {
        record = InputRecord{};
        record->deltaTime = deltaTime;
        record->listening = 0;

        for (Input type : accept_inputs) {
            record->listening |= 1 << static_cast<uint8_t>(type);
        }
    }
}

bool InputSourceRealtime::push_event(SDL_Event e) {
    bool caught = false;
    switch (e.type) {
        case SDL_EVENT_KEY_DOWN: {
            if (e.key.repeat != 0) {
                // discard repeats
                break;
            } else if (auto input_type = InputConfig::get_type_key(e.key.key);
                    input_type && listening.contains(*input_type))
            {
                events.push_back({*input_type, true, InputHandle::MAG_FULL});
                caught = true;
            }
        }
            break;
        case SDL_EVENT_KEY_UP: {
            if (auto input_type = InputConfig::get_type_key(e.key.key);
                    input_type && listening.contains(*input_type))
            {
                events.push_back({*input_type, true, InputHandle::MAG_ZERO});
                caught = true;
            }
        }
            break;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
            if (auto input_type = InputConfig::get_type_jbutton(e.gbutton.button);
                    input_type && listening.contains(*input_type))
            {
                events.push_back({*input_type, true, InputHandle::MAG_FULL});
                caught = true;
            }
        }
            break;
        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            if (auto input_type = InputConfig::get_type_jbutton(e.gbutton.button);
                    input_type && listening.contains(*input_type))
            {
                events.push_back({*input_type, true, InputHandle::MAG_ZERO});
                caught = true;
            }
        }
            break;
        case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
            JoystickAxis axis = e.gaxis.axis;
            int16_t axis_pos = e.gaxis.value;

            std::optional<JoystickAxis> alt_axis;
            int16_t alt_axis_pos = 0;

            switch (e.gaxis.axis) {
                case SDL_GAMEPAD_AXIS_LEFTX:
                    alt_axis = SDL_GAMEPAD_AXIS_LEFTY;
                    break;
                case SDL_GAMEPAD_AXIS_LEFTY:
                    alt_axis = SDL_GAMEPAD_AXIS_LEFTX;
                    break;
                case SDL_GAMEPAD_AXIS_RIGHTX:
                    alt_axis = SDL_GAMEPAD_AXIS_RIGHTY;
                    break;
                case SDL_GAMEPAD_AXIS_RIGHTY:
                    alt_axis = SDL_GAMEPAD_AXIS_RIGHTX;
                    break;
            }
            if (alt_axis) {
                alt_axis_pos = SDL_GetJoystickAxis(SDL_GetJoystickFromID(e.gaxis.which), *alt_axis);
            }

            auto opt_process_axis = [this](std::optional<Input> in_opt, bool side, uint16_t pos, uint16_t alt_pos) {
                if (in_opt) {
                    auto &data = axes[*in_opt];
                    data.positive_side = side;
                    process_axis(*in_opt, axes[*in_opt], pos, alt_pos);
                }
            };

            auto [axis1, axis2] = InputConfig::get_type_jaxis(axis);
            opt_process_axis(axis1, false, axis_pos, alt_axis_pos);
            opt_process_axis(axis2, true, axis_pos, alt_axis_pos);

            if (alt_axis) {
                auto [alt1, alt2] = InputConfig::get_type_jaxis(*alt_axis);
                opt_process_axis(alt1, false, alt_axis_pos, axis_pos);
                opt_process_axis(alt2, true, alt_axis_pos, axis_pos);
            }
        }
            break;
    }
    return caught;
}

void InputSourceRealtime::next() {

    // record current frame
    if (record && record->frame_data.size() < INPUT_RECORD_SIZE_MAX) {
        auto frame = record->frame_data.empty()
                     ? InputFrame{}
                     : record->frame_data.back();

        frame.pressed           = 0;
        frame.activation_change = 0;

        for (auto& e : events) {
            size_t type_ndx = static_cast<size_t>(e.type);

            if (e.activate_or_deactivate && e.magnitude > 0) {
                frame.pressed |= 1 << type_ndx;
            }

            frame.magnitudes[type_ndx] = e.magnitude;

            if (e.activate_or_deactivate) {
                frame.activation_change |= 1 << type_ndx;
            }
        }
        record->frame_data.push_back(frame);
    }

    events.clear();
    ++tick;
}

const std::vector<InputEvent>& InputSourceRealtime::get_events() const
{
    return events;
}

void InputSourceRealtime::set_record(const InputRecord& t_record)
{
    if (record) {
        record = t_record;
        tick = record->frame_data.size();
    }
}

const std::optional<InputRecord>& InputSourceRealtime::get_record() const
{
    return record;
}