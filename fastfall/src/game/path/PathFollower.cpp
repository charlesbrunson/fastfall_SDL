#include "fastfall/game/path/PathFollower.hpp"

namespace ff {

PathFollower::PathFollower(Path p)
    : _path(p)
{
}

void PathFollower::reset(Path p) {
    _path = p;
    timer = 0.0;
    curr_waypoint_ndx = 0;
    dist_to_next_waypoint = 0.f;
    progress = 0.f;
}

void PathFollower::update(AttachPoint& attach, secs deltaTime)
{
    // TODO later
}

bool PathFollower::at_end() const {
    return curr_waypoint_ndx == 0 || curr_waypoint_ndx == (_path.waypoints.size() - 1);
}

Vec2f PathFollower::prev_waypoint_pos() const {
    if (curr_waypoint_ndx > 0) {
        return _path.origin + _path.waypoints.at(curr_waypoint_ndx - 1);
    }
    else {
        return _path.origin + _path.waypoints.at(0);
    }
}

Vec2f PathFollower::next_waypoint_pos() const {
    return _path.origin + _path.waypoints.at(curr_waypoint_ndx);
}

Vec2f PathFollower::get_pos() const {
    float fract = dist_to_next_waypoint != 0.f ? progress / dist_to_next_waypoint : 0.f;
    return prev_waypoint_pos() + (next_waypoint_pos() - prev_waypoint_pos()) * fract;
}

Vec2f PathFollower::get_vel() const {
    return vel;
}

}