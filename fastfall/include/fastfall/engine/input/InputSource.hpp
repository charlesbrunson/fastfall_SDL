#pragma once

#include "fastfall/engine/input/Input_Def.hpp"

#include <vector>
#include <set>
#include <limits>

namespace ff {

namespace input_sets {
    extern const std::set<Input> gameplay;
}

struct InputEvent {
    Input type;
    bool activate_or_deactivate = false;
    uint8_t magnitude;
};

class InputState;

class InputSourceNull;

class InputSource {
public:
    explicit InputSource(const std::set<Input>& listen_to)
        : listening(listen_to)
    {
    };

    explicit InputSource(uint8_t listen_to)
    {
        for (uint8_t i = 0; i < std::numeric_limits<uint8_t>::digits; ++i)
        {
            bool val = listen_to & i;
            if (val)
                listening.insert(static_cast<Input>(i));
        }
    };

    virtual const std::vector<InputEvent>& get_events() const = 0;

    bool is_listening(Input in) const { return listening.contains(in); }
    const std::set<Input>& get_listening() const { return listening; }

    virtual void next() = 0;
private:
    std::set<Input> listening;
    InputState* consumer = nullptr;
};

}