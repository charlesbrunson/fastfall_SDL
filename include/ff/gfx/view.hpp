#pragma once

#include "ff/util/rect.hpp"

namespace ff {

class view {
public:
    view();
    view(vec2 t_botleft, vec2 t_size, vec2 t_scale = {1.f, 1.f});

    void set_center(vec2 t_center);
    void set_viewport(rectf t_viewport);
    void set_size(vec2 t_size);
    void set_zoom(float t_zoom);

    vec2 get_center() const;
    rectf get_viewport() const;
    vec2 get_size() const;
    float get_zoom() const;

    mat3 matrix() const;
    mat3 inv_matrix() const;

private:
    vec2 m_center;
    rectf m_viewport;
    vec2 m_size;
    float m_zoom = 1.f;
    vec2 m_scale;
};

}
