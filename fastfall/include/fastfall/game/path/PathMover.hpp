#pragma once

#include "fastfall/game/path/Path.hpp"
#include "fastfall/game/attach/AttachPoint.hpp"
#include "fastfall/util/id.hpp"


namespace ff {

enum class PathOnComplete {
    Reverse,
    Restart
};

class PathMover {
public:
    PathMover() = default;
    PathMover(Path p);

    void reset();
    void reset(const Path& p);

    void update(AttachPoint& attach, secs deltaTime);

    void set_attach_id(ID<AttachPoint> id) { _attach_id = id; };
    ID<AttachPoint> get_attach_id() const { return _attach_id; }

    bool at_start() const;
    bool at_end() const;
    Vec2f prev_waypoint_pos() const;
    Vec2f next_waypoint_pos() const;
    Vec2f get_pos() const;
    Vec2f get_vel() const;

    float speed = 0.f;
    bool stopped = false;
    PathOnComplete on_complete = PathOnComplete::Reverse;
    bool stop_on_complete = false;

    secs wait_on_start = 0.0;
    secs wait_on_way   = 0.0;
    secs wait_on_end   = 0.0;

private:
    ID<AttachPoint> _attach_id;
    Path _path;
    secs timer = 0.0;

    size_t prev_ndx = 0;
    size_t curr_ndx = 0;
    float dist_to_next_waypoint = 0.f;
    float progress = 0.f;
    Vec2f vel;
    bool at_waypoint = true;
    bool reversed = false;
};

}
