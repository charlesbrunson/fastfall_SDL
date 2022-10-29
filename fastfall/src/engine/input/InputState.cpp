#include "fastfall/engine/input/InputState.hpp"

#include "fastfall/util/log.hpp"

using namespace ff;

InputState::InputState(InputSource* source)
{
   set_source(source);
}

void InputState::set_source(InputSource* source)
{
    input_source = source;
    if (input_source) {
        auto tmp_state = input_states;
        input_states.clear();
        for (auto &in : input_source->get_listening()) {
            if (tmp_state.contains(in)) {
                input_states.emplace(in, tmp_state.at(in));
            }
            else {
                input_states.emplace(in, Input{in});
            }
        }
    }
    else {
        input_states.clear();
    }
}

void InputState::update(secs deltaTime)
{
    if (deltaTime > 0.0) {
        process_events();
        for (auto& [type, input] : input_states) {
            input.update(deltaTime);
        }
        ++input_tick;
    }
}

void InputState::process_events()
{
    if (input_source) {
        for (const auto &event: input_source->get_events()) {
            auto &input = at(event.type);

            if (event.activate_or_deactivate) {
                if (event.magnitude > 0) {
                    input.activate();
                } else {
                    input.deactivate();
                }
            }

            input.set_magnitude(event.magnitude);
        }
        input_source->next();
    }
}

// ------------------------------------------------------

bool InputState::is_listening(InputType in) const { return input_states.contains(in); }
bool InputState::is_listening(std::optional<InputType> in) const { return in && input_states.contains(*in); }

//void InputState::set_tick(size_t tick) { input_tick = tick; }

size_t InputState::get_tick() const { return input_tick; }

