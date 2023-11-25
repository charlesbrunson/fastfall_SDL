#pragma once

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/math.hpp"

#include "fastfall/engine/input/InputHandle.hpp"
#include "fastfall/render/util/View.hpp"

namespace ff {
    class MouseInput : public InputHandle {
    public:
        MouseInput() : InputHandle(Input::None)
        {};

        Vec2i mouse_press_pos;
        Vec2i mouse_release_pos;
        bool active = false;
    };
}

namespace ff::Mouse {
    enum class MButton {
        Left,
        Right
    };

    void notify_down(MButton button, Vec2i windowpos);
    void notify_up(MButton button, Vec2i windowpos);

    void reset();

    void update(secs deltaTime);

    void set_window_pos(Vec2i pos);
    void update_view(const View& view);

    Vec2i window_pos();
    Vec2f world_pos();

    bool in_view();

    extern MouseInput m1;
    extern MouseInput m2;
}