#pragma once

#include "fastfall/util/math.hpp"

namespace ff {

struct QuadID {
    int value = -1;

    bool operator==(const QuadID other) const {
        return value == other.value;
    }
    bool operator<(const QuadID other) const {
        return value < other.value;
    }

    Vec2i to_pos(Vec2u size, bool border) const {
        Vec2i pos;
        Vec2i size_min{ 0, 0 };
        Vec2i size_max{ size };
        if (border) {
            size_min -= Vec2i{ 1, 1 };
            size_max += Vec2i{ 1, 1 };
        }
        pos.y = value / (size_max.x);
        pos.x = value - (pos.y * (size_max.x - size_min.x));
        pos += size_min;
        return pos;
    }
};


}

