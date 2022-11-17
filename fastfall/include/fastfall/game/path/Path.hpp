#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/resource/asset/LevelAssetTypes.hpp"

#include <vector>


namespace ff {

enum class PathOnComplete {
    Reverse,
    Restart
};

struct Path {
    Path() = default;
    Path(Vec2f t_origin, std::vector<Vec2f> t_waypoints, float t_speed);
    Path(const ObjectLevelData* data);

    Vec2f origin{};
    std::vector<Vec2f> waypoints{};
    float speed = 0.f;
    secs wait_on_start = 0.0;
    secs wait_on_way   = 0.0;
    secs wait_on_end   = 0.0;
    bool stop_on_complete = false;
    PathOnComplete on_complete = PathOnComplete::Reverse;
};

}