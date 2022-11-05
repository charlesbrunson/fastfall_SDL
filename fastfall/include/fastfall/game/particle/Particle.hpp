#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/engine/time/time.hpp"

namespace ff {
    // base particle class
    struct Particle {
        Vec2f position      = {};
        Vec2f prev_position = {};
        Vec2f velocity      = {};
        Vec2f accel         = {};
        secs  lifetime      = 0.0;
        bool  is_alive      = true;
    };
}