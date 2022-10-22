#pragma once

#include "fastfall/engine/input/Input_Def.hpp"

#include <vector>
#include <set>

namespace ff {

namespace input_sets {
    extern const std::set<InputType> gameplay;
}

struct InputEvent {
    InputType type;
    bool activate_or_deactivate = false;
    uint8_t magnitude;
};

class InputSource {
public:
    InputSource(const std::set<InputType>& listen_to)
        : listening(listen_to)
    {
    };

    virtual const std::vector<InputEvent>& get_events() const = 0;

    bool is_listening(InputType in) const { return listening.contains(in); }
    const std::set<InputType>& get_listening() const { return listening; }

private:
    std::set<InputType> listening;
};

class InputSourceNull : InputSource {
public:
    InputSourceNull() : InputSource({}) {};
    const std::vector<InputEvent>& get_events() const override { return null_events; }
private:
    std::vector<InputEvent> null_events;
};

}