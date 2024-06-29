#pragma once

#include "vertex_buffer.hpp"

#include "ff/util/math.hpp"

namespace ff {

class vertex_array {
    vertex_array();
public:
    vertex_array(const vertex_array&) = delete;
    vertex_array& operator=(const vertex_array&) = delete;
    vertex_array(vertex_array&&) = default;
    vertex_array& operator=(vertex_array&&) = default;
    ~vertex_array();

private:

};

}