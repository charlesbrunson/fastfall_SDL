#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/engine/time/time.hpp"

namespace ff {
    // base particle class
    struct Particle {
        size_t id           = 0;
        Vec2f position      = {};
        Vec2f prev_position = {};
        Vec2f velocity      = {};
        secs  lifetime      = 0.0;
        // float interp_offset = 0.f;
        bool  is_alive      = true;
    };
}