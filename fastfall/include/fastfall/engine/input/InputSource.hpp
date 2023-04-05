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

class InputState;

class InputSourceNull;

class InputSource {
public:
    InputSource(const std::set<InputType>& listen_to)
        : listening(listen_to)
    {
    };

    virtual const std::vector<InputEvent>& get_events() const = 0;

    bool is_listening(InputType in) const { return listening.contains(in); }
    const std::set<InputType>& get_listening() const { return listening; }

    virtual void next() = 0;
private:
    std::set<InputType> listening;
    InputState* consumer = nullptr;
};

}