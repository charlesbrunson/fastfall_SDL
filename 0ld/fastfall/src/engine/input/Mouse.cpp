#include "fastfall/engine/input/Mouse.hpp"

namespace ff::Mouse {
    MouseInput m1;
    MouseInput m2;
    Vec2i _window_pos;
    Vec2f _world_pos;
    bool mouse_in_view;

    void notify_down(MButton button, Vec2i windowpos) {
        MouseInput *in = nullptr;
        if (button == MButton::Left) {
            in = &Mouse::m1;
        } else if (button == MButton::Right) {
            in = &Mouse::m2;
        }
        if (in) {
            if (!in->active) {
                in->activate();
                in->mouse_press_pos = windowpos;
            }
            in->active = true;
        }
    }
    void notify_up(MButton button, Vec2i windowpos) {
        MouseInput *in = nullptr;
        if (button == MButton::Left) {
            in = &Mouse::m1;
        } else if (button == MButton::Right) {
            in = &Mouse::m2;
        }
        if (in) {
            in->deactivate();
            in->mouse_release_pos = windowpos;
            in->active = false;
        }
    }

    void reset() {
        m1.reset();
        m2.reset();
    }

    void update(secs deltaTime) {
        m1.update(deltaTime);
        m2.update(deltaTime);
    }

    void set_window_pos(Vec2i pos) {
        _window_pos = pos;
    }

    void update_view(const View& view) {
        glm::vec4 viewport = view.getViewport();
        glm::fvec2 vsize = view.getSize();
        glm::fvec2 vzoom = glm::fvec2(viewport[2] / vsize.x, viewport[3] / vsize.y);
        glm::fvec2 viewcenter{ viewport[0] + viewport[2] / 2, viewport[1] + viewport[3] / 2 };

        _world_pos = Vec2f{
                ((float)_window_pos.x - viewcenter.x) / vzoom.x,
                ((float)_window_pos.y - viewcenter.y) / vzoom.y
        };
        _world_pos += Vec2f{ view.getCenter() };

        mouse_in_view = _window_pos.x >= viewport[0]
                     && _window_pos.x < (viewport[0] + viewport[2])
                     && _window_pos.y >= viewport[1]
                     && _window_pos.y < (viewport[1] + viewport[3]);
    }

    Vec2i window_pos() {
        return _window_pos;
    }
    Vec2f world_pos() {
        return _world_pos;
    }
    bool in_view() {
        return mouse_in_view;
    }
}
