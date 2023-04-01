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

        speed            = data->get_prop_or("speed",         0.f);
        wait_on_start    = data->get_prop_or("wait_on_start", 0.f);
        wait_on_way      = data->get_prop_or("wait_on_way",   0.f);
        wait_on_end      = data->get_prop_or("wait_on_end",   0.f);
        stop_on_complete = data->get_prop_or("wait_on_end",   false);

        if (auto p = data->get_prop_opt<std::string>("on_complete")) {
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