#pragma once

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


struct uniform {
    char parameter_name[32];
    unsigned location;

    uniform_type type;
    uint8_t extent1;
    uint8_t extent2;

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