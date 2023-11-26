#include "fastfall/game/phys/collidable/SurfaceFollow.hpp"

#include "fastfall/util/log.hpp"

using namespace ff;


bool SurfaceFollow::compare_paths(float travel_dir, const surface_path& from, const surface_path& pick, const surface_path& candidate)
{
    float curr_dist = math::dist(pick.start_pos,      from.start_pos);
    float cand_dist = math::dist(candidate.start_pos, from.start_pos);

    if (cand_dist < curr_dist) {
        return true;
    }

    Angle ang       = math::angle(from.surface_line);
    Angle curr_ang  = math::angle(pick.surface_line)      - ang;
    Angle cand_ang  = math::angle(candidate.surface_line) - ang;

    return travel_dir > 0.f ? cand_ang < curr_ang : cand_ang > curr_ang;
}

Vec2f get_travel_offset(Linef line, Vec2f body_size) {
    if (math::is_vertical(line)) {
        // is wall
        return Vec2f{ (body_size.x * 0.5f) * (line.p1.y < line.p2.y ? 1.f : -1.f), 0.f };
    }
    else if (line.p1.x > line.p2.x) {
        // is ceil
        return Vec2f{ 0.f, body_size.y };
    }
    return Vec2f{};
}

SurfaceFollow::SurfaceFollow(Linef init_path, Vec2f init_pos, float travel_dir, float distance, SurfaceTracker::applicable_ang_t angle_ranges, Angle max_angle, Vec2f collidable_size)
    : curr_path     { 0, init_path, math::shift(init_path, get_travel_offset(init_path, collidable_size)), init_pos}
    , travel_dir    { travel_dir }
    , travel_dist   { distance }
    , angle_range   { angle_ranges }
    , angle_max     { max_angle }
    , body_size     { collidable_size }
{
    path_taken.push_back(curr_path);
}

std::optional<SurfaceFollow::surface_id>
SurfaceFollow::add_surface(Linef path)
{
    if (auto surf_path = valid_surface(path, candidate_paths.size())) {
        candidate_paths.push_back(*surf_path);
        return surf_path->id;
    }
    return{};
}

void SurfaceFollow::reset() {
    candidate_paths.clear();
}

std::optional<SurfaceFollow::surface_id>
SurfaceFollow::pick_surface_to_follow()
{
    if (travel_dir == 0.f)
        return {};

    const surface_path* curr_pick = nullptr;
    Vec2f curr_dir = travel_dir * math::tangent(curr_path.surface_line);

    for (auto& candidate : candidate_paths)
    {
        auto dir_cand = travel_dir > 0.f ? candidate.surface_line : candidate.surface_line.reverse();

        // skip surfaces we've started at the end of
        if (candidate.start_pos == dir_cand.p2) {
            continue;
        }
        // skip surfaces we've already been on
        for (auto& visited : path_taken) {
            if (visited.surface_line == candidate.surface_line) {
                continue;
            }
        }
        // skip if surface intersect is on or behind us
        float dir_dot = math::dot(curr_dir, candidate.start_pos - curr_path.start_pos);
        if (dir_dot <= 0) {
            continue;
        }
        // pick surface if it's been than current pick
        if (!curr_pick || compare_paths(travel_dir, curr_path, *curr_pick, candidate)) {
            curr_pick = &candidate;
        }
    }
    return (curr_pick ? std::make_optional(curr_pick->id) : std::nullopt);
}

SurfaceFollow::travel_result
SurfaceFollow::travel_to(surface_id id)
{
    const surface_path& path = candidate_paths[id];
    Vec2f unit = (path.start_pos - curr_path.start_pos).unit();
    float dist = math::dist(curr_path.start_pos, path.start_pos);

    Vec2f npos;
    bool can_get_to_new_path = dist <= travel_dist;

    if (!can_get_to_new_path) {
        npos = curr_path.start_pos + (travel_dist * unit);
        travel_dist = 0.f;
    }
    else {
        npos = path.start_pos;
        travel_dist -= dist;
        curr_path = path;
        path_taken.push_back(path);
    }

    reset();

    return {
        .path           = curr_path,
        .dist           = travel_dist,
        .pos            = npos,
        .on_new_surface = can_get_to_new_path
    };
}

SurfaceFollow::travel_result SurfaceFollow::finish() {
    Vec2f unit = math::tangent(curr_path.surface_line).unit() * travel_dir;

    Vec2f npos = curr_path.start_pos + (unit * travel_dist);

    travel_dist = 0.f;

    path_taken.push_back(curr_path);

    reset();

    return {
        .path           = curr_path,
        .dist           = travel_dist,
        .pos            = npos,
        .on_new_surface = false
    };
}

std::optional<SurfaceFollow::surface_path>
SurfaceFollow::valid_surface(Linef path, surface_id id) const
{
    if (path == curr_path.surface_line) {
        // this is the surface we're currently on
        return {};
    }



    Linef travel_line = math::shift(path, get_travel_offset(path, body_size));

    // reverse lines if travel_dir < 0.f
    auto dir_curr = travel_dir > 0.f ? curr_path.travel_line : curr_path.travel_line.reverse();
    auto dir_path = travel_dir > 0.f ? travel_line : travel_line.reverse();

    Vec2f inter = dir_curr.p2 == dir_path.p1
                  ? dir_curr.p2
                  : math::intersection(dir_path, dir_curr);

    std::optional<Vec2f> intersect;
    if (inter != Vec2f{NAN, NAN})
    {
        // make sure the intersection lies on both lines
        if (   math::line_has_point(dir_curr, inter)
            && math::line_has_point(dir_path, inter))
        {
            // surface connects at the end of curr_path
            bool connect_at_end = inter == dir_curr.p2;

            // surface forms a ramp on curr_path
            bool connect_in_middle = math::angle(dir_path) - math::angle(dir_curr) < 0.f;

            if (connect_at_end || connect_in_middle) {
                intersect = inter;
            }
        }
    }
    else if (math::collinear(dir_path, dir_curr))
    {
        bool is_ahead = math::dot(dir_path.p1 - dir_curr.p1, math::tangent(dir_path)) > 0;
        Vec2f collinear_intersect = dir_path.p2;

        if (is_ahead && math::line_has_point(dir_curr, collinear_intersect)) {
            intersect = collinear_intersect;
        }
    }

    if (!intersect) {
        return {};
    }

    Angle next_ang = math::angle(math::tangent(dir_path));
    Angle curr_ang = math::angle(math::tangent(dir_curr));
    Angle diff = next_ang - curr_ang;

    Angle normal = math::angle(math::normal(path));
    bool in_range = angle_range.within_range(normal);
    bool in_max_angle = abs(diff.degrees()) < abs(angle_max.degrees());

    if (in_range && in_max_angle) {
        return surface_path {
            .id           = id,
            .surface_line = path,
            .travel_line  = travel_line,
            .start_pos    = *intersect,
            .angle        = next_ang,
            .diff_angle   = diff
        };
    }
    return {};
}
