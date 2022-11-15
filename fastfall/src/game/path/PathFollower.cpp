#include "fastfall/game/path/PathFollower.hpp"

#include <algorithm>

namespace ff {

PathFollower::PathFollower(Path p)
    : _path(std::move(p))
{
    timer = wait_on_end;
}

void PathFollower::reset(Path p) {
    _path = std::move(p);
    timer = 0.0;
    curr_waypoint_ndx = 0;
    dist_to_next_waypoint = 0.f;
    progress = 0.f;
}

void PathFollower::update(AttachPoint& attach, secs deltaTime)
{
    Vec2f prev_pos = get_pos();
    if (!_path.waypoints.empty()) {
        if (timer > 0.0) {
            timer = std::max(0.0, timer - deltaTime);
        } else if (speed != 0.f && !stopped) {
            timer = 0.0;
            progress = std::min(dist_to_next_waypoint, progress + speed * (float) deltaTime);
            if (progress == dist_to_next_waypoint) {
                timer = (at_end() ? wait_on_end : wait_on_way);

                progress = 0.f;

                if (at_end() && curr_waypoint_ndx != 0) {
                    switch (on_complete) {
                        case PathOnComplete::Reverse:
                            curr_waypoint_ndx = 1;
                            std::reverse(_path.waypoints.begin(), _path.waypoints.end());
                            break;
                        case PathOnComplete::Restart:
                            curr_waypoint_ndx = 1;
                            break;
                    }

                    if (stop_on_complete)
                        stopped = true;
                }
                else {
                    ++curr_waypoint_ndx;
                    dist_to_next_waypoint = (next_waypoint_pos() - prev_waypoint_pos()).magnitude();
                }
            }
        }
    }

    LOG_INFO("{} {}: {} / {}", timer, curr_waypoint_ndx, progress, dist_to_next_waypoint);

    attach.set_pos(get_pos());
    vel = (get_pos() - prev_pos) / deltaTime;
    attach.set_vel(get_vel());
}

bool PathFollower::at_end() const {
    return curr_waypoint_ndx == 0 || curr_waypoint_ndx == (_path.waypoints.size() - 1);
}

Vec2f PathFollower::prev_waypoint_pos() const {
    if (!_path.waypoints.empty()) {
        if (curr_waypoint_ndx > 0) {
            return _path.origin + _path.waypoints.at(curr_waypoint_ndx - 1);
        } else {
            return _path.origin + _path.waypoints.at(0);
        }
    }
    else {
        return _path.origin;
    }
}

Vec2f PathFollower::next_waypoint_pos() const {
    if (!_path.waypoints.empty()) {
        return _path.origin + _path.waypoints.at(curr_waypoint_ndx);
    }
    else {
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