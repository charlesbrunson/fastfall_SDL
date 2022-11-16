#include "fastfall/game/path/PathFollower.hpp"

#include <algorithm>

namespace ff {

PathFollower::PathFollower(Path p)
    : _path(std::move(p))
{
    timer = wait_on_start;
}

void PathFollower::reset(Path p) {
    _path = std::move(p);
    timer = wait_on_start;
    curr_ndx = 0;
    prev_ndx = 0;
    dist_to_next_waypoint = 0.f;
    progress = 0.f;
}

void PathFollower::update(AttachPoint& attach, secs deltaTime)
{
    Vec2f prev_pos = get_pos();
    auto next_segment = [&, this]() {
        if (curr_ndx < _path.waypoints.size() - 1) {
            ++curr_ndx;
            dist_to_next_waypoint = (next_waypoint_pos() - prev_waypoint_pos()).magnitude();
            progress = 0.f;
        }
        else {
            if (stop_on_complete) {
                stopped = true;
            }
            else {
                curr_ndx = 0;
                prev_ndx = 0;
                timer = wait_on_start;

                switch (on_complete) {
                    case PathOnComplete::Restart:
                        prev_pos = get_pos();
                        attach.teleport(prev_pos);
                        break;
                    case PathOnComplete::Reverse:
                        std::reverse(_path.waypoints.begin(), _path.waypoints.end());
                        break;
                }
            }
        }
    };

    if (curr_ndx == prev_ndx) {
        if (timer > 0.0) {
            timer = std::max(0.0, timer - deltaTime);
        }
        else {
            next_segment();
        }
    }
    else if (!stopped) {
        progress += speed * (float)deltaTime;
        if (progress >= dist_to_next_waypoint) {
            ++prev_ndx;
            progress -= dist_to_next_waypoint;

            if (at_start())     { timer = wait_on_start; }
            else if (at_end())  { timer = wait_on_end; }
            else                { timer = wait_on_way; }

            if (timer == 0.0) {
                next_segment();
            }
            else {
                progress = 0.f;
            }
        }
    }

    attach.set_pos(get_pos());
    vel = (get_pos() - prev_pos) / deltaTime;
    attach.set_vel(get_vel());
}

bool PathFollower::at_start() const {
    return prev_ndx == curr_ndx == 0;
}

bool PathFollower::at_end() const {
    return prev_ndx == curr_ndx == (_path.waypoints.size() - 1);
}

Vec2f PathFollower::prev_waypoint_pos() const {
    if (!_path.waypoints.empty()) {
        return _path.origin + _path.waypoints.at(prev_ndx);
    } else {
        return _path.origin;
    }
}

Vec2f PathFollower::next_waypoint_pos() const {
    if (!_path.waypoints.empty()) {
        return _path.origin + _path.waypoints.at(curr_ndx);
    } else {
        return _path.origin;
    }
}

Vec2f PathFollower::get_pos() const {
    float fract = dist_to_next_waypoint != 0.f ? progress / dist_to_next_waypoint : 0.f;
    return prev_waypoint_pos() + (next_waypoint_pos() - prev_waypoint_pos()) * fract;
}

Vec2f PathFollower::get_vel() const {
    return vel;
}

}