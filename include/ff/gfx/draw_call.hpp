#pragma once

#include "ff/gfx/vertex_array.hpp"

#include <variant>

namespace ff {

struct draw_call {
    vertex_array_base& varr;
};

}