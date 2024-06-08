#pragma once

#include "ff/util/math.hpp"
#include "ff/util/rect.hpp"

namespace ff {

class view {
public:
    view();
    view(vec2f botleft, vec2f size, vec2f scale = {1.f, 1.f});

    void set_center(vec2f center);
    void set_viewport(rectf viewport);
    void set_size(vec2f size);
    void set_zoom(float zoom);

    vec2f get_center() const;
    rectf get_viewport() const;
    vec2f get_size() const;
    float get_zoom() const;

    mat3f matrix() const;
    mat3f inv_matrix() const;

private:
    vec2f center;
    rectf viewport;
    vec2f size;
    float zoom = 1.f;

    vec2f scale;
};

}
