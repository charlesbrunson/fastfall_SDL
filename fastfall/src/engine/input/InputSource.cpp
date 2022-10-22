#include "fastfall/engine/input/InputSource.hpp"

using namespace ff;

namespace ff::input_sets {
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