#include <utility>

#include "fastfall/game/path/Path.hpp"


namespace ff {

Path::Path(Vec2f t_origin, std::vector<Vec2f> t_waypoints, float t_speed)
    : origin(t_origin)
    , waypoints(std::move(t_waypoints))
    , speed(t_speed)
{
}

Path::Path(const LevelObjectData* data)
    : origin(data ? Vec2f{data->area.topleft()} : Vec2f{})
{
    if (data) {
        std::transform(
            data->points.cbegin(),
            data->points.cend(),
            std::back_inserter(waypoints),
            [](auto p) { return Vec2f{ p }; }
        );

        if (auto p = data->optPropAsFloat("speed")) { speed = *p; }
        if (auto p = data->optPropAsFloat("wait_on_start")) { wait_on_start = *p; }
        if (auto p = data->optPropAsFloat("wait_on_way")) { wait_on_way = *p; }
        if (auto p = data->optPropAsFloat("wait_on_end")) { wait_on_end = *p; }
        if (auto p = data->optPropAsBool("stop_on_complete")) { stop_on_complete = *p; }

        if (auto p = data->optPropAsString("on_complete")) {
            std::string str = std::move(*p);
            if (str == "restart") {
                on_complete = PathOnComplete::Restart;
            } else if (str == "reverse") {
                on_complete = PathOnComplete::Reverse;
            }
        }
    }
}

}