#pragma once

#include "../util/math.hpp"
#include "color.hpp"

namespace ff {

struct vertex {
    vec3f pos;
    vec2f tex_pos;
    color col;
};

}