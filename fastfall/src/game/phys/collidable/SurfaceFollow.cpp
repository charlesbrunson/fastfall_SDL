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

    Angle ang       = math::angle(from.line);
    Angle curr_ang  = math::angle(pick.line)      - ang;
    Angle cand_ang  = math::angle(candidate.line) - ang;

    return travel_dir > 0.f ? cand_ang < curr_ang : cand_ang > curr_ang;
}

SurfaceFollow::SurfaceFollow(Linef init_path, Vec2f init_pos, float travel_dir, float distance, SurfaceTracker::applicable_ang_t angle_ranges, Angle max_angle)
    : curr_path     { 0, init_path, init_pos}
    , travel_dir    { travel_dir }
    , travel_dist   { distance }
    , angle_range   { angle_ranges }
    , angle_max     { max_angle }
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
    Vec2f curr_dir = travel_dir * math::tangent(curr_path.line);

    for (auto& candidate : candidate_paths)
    {
        if ((travel_dir > 0.f && candidate.start_pos == candidate.line.p2)
         || (travel_dir < 0.f && candidate.start_pos == candidate.line.p1))
        {
            LOG_INFO("\tpick: start_pos is line end {}->{}", candidate.line.p1, candidate.line.p2);
            continue;
        }

        for (auto& visited : path_taken) {
            if (visited.line == candidate.line) {
                LOG_INFO("\tpick: prev visited {}->{}", candidate.line.p1, candidate.line.p2);
                continue;
            }
        }

        // surface intersect is on or behind us
        float dir_dot = math::dot(curr_dir, candidate.start_pos - curr_path.start_pos);
        if (dir_dot <= 0) {
            LOG_INFO("\tpick: is behind {}->{}, dot:{}, dir:{}", candidate.line.p1, candidate.line.p2, dir_dot, curr_dir);
            continue;
        }

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
    Vec2f unit = math::tangent(curr_path.line).unit() * travel_dir;

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
    if (path == curr_path.line) {
        LOG_INFO("\terr -- same line");
        return {};
    }

    Vec2f inter;
    if ((travel_dir < 0.f && curr_path.line.p1 == path.p2)
     || (travel_dir > 0.f && curr_path.line.p2 == path.p1))
    {
        inter = travel_dir < 0.f ? curr_path.line.p1 : curr_path.line.p2;
    }
    else {
        inter  = math::intersection(path, curr_path.line);
    }

    Rectf bounds = math::line_bounds(curr_path.line);

    std::optional<Vec2f> intersect;

    if (inter != Vec2f{NAN, NAN})
    {
        if (inter == (travel_dir < 0.f ? curr_path.line.p1 : curr_path.line.p2)) {
            // connected at the end
            intersect = inter;
        }
        else if (bounds.contains(inter)
            && (travel_dir > 0.f ? math::angle(path) - math::angle(curr_path.line) < 0.f : math::angle(path) - math::angle(curr_path.line) > 0.f))
        {
            // connects in the middle somewhere
            // remove if below the line
            intersect = inter;
        }
    }
    else if (math::collinear(path, curr_path.line))
    {
        //LOG_INFO("\t\thas collinear");
        Vec2f p1 = travel_dir > 0.f ? curr_path.line.p2 : curr_path.line.p1;
        Vec2f p2 = travel_dir > 0.f ? path.p2 : path.p1;
        bool is_ahead = math::dot(p2 - p1, travel_dir * math::tangent(curr_path.line)) > 0;

        if (is_ahead && bounds.touches(math::line_bounds(path))) {
            //LOG_INFO("\t\tcollinear in bounds");
            intersect = (travel_dir > 0.f ? path.p1 : path.p2);
        }
    }

    if (!intersect) {
        LOG_INFO("\terr -- no intersect");
        return {};
    }

    Angle backwards = travel_dir < 0.f ? Angle::Degree(180.f) : Angle{};
    Angle next_ang = math::angle(math::tangent(path)) + backwards;
    Angle curr_ang = math::angle(math::tangent(curr_path.line)) + backwards;
    Angle diff = next_ang - curr_ang;

    Angle normal = math::angle(math::normal(path));
    bool in_range = angle_range.within_range(normal);
    bool in_max_angle = abs(diff.degrees()) < abs(angle_max.degrees());

    if (in_range && in_max_angle) {
        SurfaceFollow::surface_path out_path;
        out_path.id         = id;
        out_path.line       = path;
        out_path.start_pos  = *intersect;
        out_path.angle      = next_ang;
        out_path.diff_angle = diff;
        return out_path;
    }

    LOG_INFO("\terr -- invalid angle {} && {}", in_range, in_max_angle);
    return {};
}
