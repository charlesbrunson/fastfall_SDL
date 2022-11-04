#include "fastfall/engine/input/InputSource.hpp"

#include "fastfall/engine/input/InputState.hpp"

namespace ff {

namespace input_sets {
    const std::set<InputType> gameplay = {
            InputType::UP,
            InputType::LEFT,
            InputType::DOWN,
            InputType::RIGHT,
            InputType::JUMP,
            InputType::DASH,
            InputType::ATTACK,
    };
}

}