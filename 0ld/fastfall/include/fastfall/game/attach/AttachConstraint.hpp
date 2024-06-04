#pragma once

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/math.hpp"

#include <functional>

namespace ff {
    class AttachPoint;
    using AttachConstraint = std::function<void(AttachPoint&, const AttachPoint&, Vec2f, secs)>;


    AttachConstraint makeSpringConstraint(Vec2f spring, Vec2f damping, float max_radius = FLT_MAX);

}