#include "fastfall/engine/input/InputState.hpp"

#include "fastfall/util/log.hpp"

#include "fastfall/engine/input/InputConfig.hpp"

using namespace ff;

InputState::InputState() {
    InputConfig::add_listener(*this);
}

InputState::InputState(InputSource* source)
{
    set_source(source);
    InputConfig::add_listener(*this);
}

InputState::InputState(const InputState& st) {
    input_source = st.input_source;
    input_states = st.input_states;
    input_tick = st.input_tick;
    InputConfig::add_listener(*this);
}

InputState::InputState(InputState&& st) noexcept {
    input_source = st.input_source;
    input_states = std::move(st.input_states);
    input_tick = st.input_tick;
    InputConfig::add_listener(*this);
}

InputState::~InputState() {
    InputConfig::remove_listener(*this);
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
        //input_source->next();
    }
}

// ------------------------------------------------------

bool InputState::is_listening(InputType in) const { return input_states.contains(in); }
bool InputState::is_listening(std::optional<InputType> in) const { return in && input_states.contains(*in); }

void InputState::notify_unbind(InputType in) {
    if (is_listening(in)) {
        at(in).reset();
    }
}

//void InputState::set_tick(size_t tick) { input_tick = tick; }

size_t InputState::get_tick() const { return input_tick; }

