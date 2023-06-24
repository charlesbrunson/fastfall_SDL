#pragma once

#include "fastfall/engine/input/Input_Def.hpp"

#include <vector>
#include <set>
#include <limits>

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
    explicit InputSource(const std::set<InputType>& listen_to)
        : listening(listen_to)
    {
    };

    explicit InputSource(uint8_t listen_to)
    {
        for (uint8_t i = 0; i < std::numeric_limits<uint8_t>::digits; ++i)
        {
            bool val = listen_to & i;
            if (val)
                listening.insert(static_cast<InputType>(i));
        }
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