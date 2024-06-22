#pragma once

#include "glm/glm.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <span>

namespace ff {

enum class uniform_type {
    Int,
    Uint,
    Float,
};

enum class uniform_format {
    Vec1,
    Vec2,
    Vec3,
    Vec4,

    Mat2x2,
    Mat3x3,
    Mat4x4,

    Mat2x3,
    Mat3x2,
    Mat2x4,
    Mat4x2,
    Mat3x4,
    Mat4x3,
};

struct uniform {
    char parameter_name[32];
    unsigned location;

    uniform_type type;
    uniform_format format;
    unsigned count;
    bool transpose = false;
};

class shader {
public:

    shader(std::string_view vert_src, std::string_view frag_src);
    ~shader();

    [[nodiscard]] const std::vector<uniform>& get_uniforms() const { return uniforms; }

private:
    std::vector<uniform> uniforms;
};

}