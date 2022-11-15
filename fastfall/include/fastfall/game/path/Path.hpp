#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/resource/asset/LevelAssetTypes.hpp"

#include <vector>


namespace ff {

struct Path {
    Path(Vec2f t_origin, std::vector<Vec2f> t_waypoints);
    Path(const ObjectLevelData* data);

    Vec2f origin;
    std::vector<Vec2f> waypoints;
};

}