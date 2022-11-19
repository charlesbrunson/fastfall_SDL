#pragma once

#include "fastfall/game/path/Path.hpp"
#include "fastfall/game/attach/AttachPoint.hpp"
#include "fastfall/util/id.hpp"


namespace ff {


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

    void set_path_offset(Vec2f t_off) { path_offset = t_off; }
    Vec2f get_path_offset() const { return path_offset; }

    const Path& get_path() const { return _path; }

    bool is_stopped() const { return stopped; }
    void set_stopped(bool stop) { stopped = stop; }

private:
    ID<AttachPoint> _attach_id;
    Path _path;
    secs timer = 0.0;

    size_t prev_ndx = 0;
    size_t curr_ndx = 0;
    float dist_to_next_waypoint = 0.f;
    float progress = 0.f;
    Vec2f vel;
    Vec2f path_offset;
    bool reversed = false;
    bool stopped = false;
};

}
