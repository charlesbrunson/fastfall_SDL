#pragma once

#include "color.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace ff {

struct vertex {
    glm::vec<3, float> pos;
    glm::vec<2, float> tex_pos;
    color col;
};

}