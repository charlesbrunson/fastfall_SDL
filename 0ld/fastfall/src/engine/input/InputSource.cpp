#include "fastfall/engine/input/InputSource.hpp"

#include "fastfall/engine/input/InputState.hpp"

namespace ff {

namespace input_sets {
    const std::set<Input> gameplay = {
            Input::Up,
            Input::Left,
            Input::Down,
            Input::Right,
            Input::Jump,
            Input::Dash,
            Input::Attack,
    };
}

}