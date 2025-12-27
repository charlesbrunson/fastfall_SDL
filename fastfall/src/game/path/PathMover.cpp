#include "fastfall/game/path/PathMover.hpp"

#include "fastfall/game/World.hpp"
#include "fastfall/game/imgui_component.hpp"

#include <algorithm>
#include <utility>

namespace ff {

PathMover::PathMover(Path p)
    : _path(std::move(p))
{
    timer = _path.wait_on_start;
}

void PathMover::reset(const Path& p) {
    _path = p;
    timer = _path.wait_on_start;
    curr_ndx = 0;
    prev_ndx = 0;
    dist_to_next_waypoint = 0.f;
    progress = 0.f;
    reversed = false;
}

void PathMover::reset() {
    if (reversed) {
        std::reverse(_path.waypoints.begin(), _path.waypoints.end());
        reversed = false;
    }

    timer = _path.wait_on_start;
    curr_ndx = 0;
    prev_ndx = 0;
    dist_to_next_waypoint = 0.f;
    progress = 0.f;
}

void PathMover::update(AttachPoint& attach, secs deltaTime)
{

    if (deltaTime > 0.0) {
        Vec2f prev_pos = get_pos();
        auto next_segment = [&, this]() {
            // are we at the end? do something
            if (curr_ndx == _path.waypoints.size() - 1) {
                if (_path.stop_on_complete) {
                    stopped = true;
                } else {
                    curr_ndx = 0;
                    prev_ndx = 0;
                    timer = _path.wait_on_start;

                    switch (_path.on_complete) {
                        case PathOnComplete::Restart:
                            prev_pos = get_pos();
                            attach.teleport(prev_pos);
                            break;
                        case PathOnComplete::Reverse:
                            std::reverse(_path.waypoints.begin(), _path.waypoints.end());
                            reversed = !reversed;
                            break;
                    }
                }
            }
            // if we're not at the end
            if (curr_ndx < _path.waypoints.size() - 1) {
                ++curr_ndx;
                dist_to_next_waypoint = math::magnitude(next_waypoint_pos() - prev_waypoint_pos());
                //progress = 0.f;
            }
        };

        if (curr_ndx == prev_ndx) {
            if (timer > 0.0) {
                timer = std::max(0.0, timer - deltaTime);
            } else {
                next_segment();
            }
        } else if (!stopped) {
            progress += _path.speed * speed_multiplier * (float) deltaTime;
            if (progress >= dist_to_next_waypoint) {
                ++prev_ndx;
                progress -= dist_to_next_waypoint;

                if (at_start()) { timer = _path.wait_on_start; }
                else if (at_end()) { timer = _path.wait_on_end; }
                else { timer = _path.wait_on_way; }

                if (timer == 0.0) {
                    next_segment();
                }
                else {
                    progress = 0.f;
                }
            }
        }

        attach.set_pos(get_pos());

        vel = (get_pos() - prev_pos) / static_cast<float>(deltaTime);

        attach.set_parent_vel(vel);
    }
}

bool PathMover::at_start() const {
    return prev_ndx == curr_ndx == 0;
}

bool PathMover::at_end() const {
    return prev_ndx == curr_ndx == (_path.waypoints.size() - 1);
}

Vec2f PathMover::prev_waypoint_pos() const {
    if (!_path.waypoints.empty()) {
        return path_offset + _path.origin + _path.waypoints.at(prev_ndx);
    } else {
        return path_offset + _path.origin;
    }
}

Vec2f PathMover::next_waypoint_pos() const {
    if (!_path.waypoints.empty()) {
        return path_offset + _path.origin + _path.waypoints.at(curr_ndx);
    } else {
        return path_offset + _path.origin;
    }
}

Vec2f PathMover::get_pos() const {
    float fract = dist_to_next_waypoint != 0.f ? progress / dist_to_next_waypoint : 0.f;
    return prev_waypoint_pos() + (next_waypoint_pos() - prev_waypoint_pos()) * fract;
}

Vec2f PathMover::get_vel() const {
    return vel;
}

void imgui_component(World& w, ID<PathMover> id) {

    auto& cmp = w.at(id);
    auto attach_id = cmp.get_attach_id();

    ImGui::Text("Attach ID: ");
    ImGui::SameLine();
    imgui_component_ref(w, attach_id);

    ImGui::Text("At Start: %s", cmp.at_start() ? "Yes" : "No");
    ImGui::Text("At End: %s", cmp.at_end() ? "Yes" : "No");
    auto prev_point = cmp.prev_waypoint_pos();
    ImGui::Text("Prev Point: %3.2f, %3.2f", prev_point.x, prev_point.y);
    auto next_point = cmp.next_waypoint_pos();
    ImGui::Text("Next Point: %3.2f, %3.2f", next_point.x, next_point.y);
    auto pos = cmp.get_pos();
    ImGui::Text("Curr Pos: %3.2f, %3.2f", pos.x, pos.y);
    auto vel = cmp.get_vel();
    ImGui::Text("Curr Vel: %3.2f, %3.2f", vel.x, vel.y);
    auto path_offset = cmp.get_path_offset();
    ImGui::Text("Path offset: %3.2f, %3.2f", path_offset.x, path_offset.y);
    auto is_stopped = cmp.is_stopped();
    ImGui::Text("Is Stopped: %s", is_stopped ? "Yes" : "No");
    auto& path = cmp.get_path();
    if (ImGui::TreeNode((void*)(&path), "Path"))
    {
        if (ImGui::TreeNode((void *) (&path.waypoints), "Path Points (%d)", (unsigned)path.waypoints.size())) {
            for (auto &point: path.waypoints) {
                ImGui::Text("%3.2f, %3.2f", point.x, point.y);
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }

    ImGui::DragFloat("Speed Multiplier", &cmp.speed_multiplier, 0.1f, 0.f, 10.f);
}

}