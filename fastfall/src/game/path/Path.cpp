#include "fastfall/game/path/Path.hpp"

namespace ff {

Path::Path(Vec2f t_origin, std::vector<Vec2f> t_waypoints)
    : origin(t_origin)
    , waypoints(t_waypoints)
{
}

Path::Path(const ObjectLevelData* data)
    : origin(data ? Vec2f{data->position} : Vec2f{})
{
    if (data) {
        std::transform(
            data->points.cbegin(),
            data->points.cend(),
            std::back_inserter(waypoints),
            [](auto p) { return Vec2f{p}; });
    }
}

}