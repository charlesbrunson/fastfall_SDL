#include "fastfall/game/phys/collidable/SurfaceFollow.hpp"

#include "fastfall/util/log.hpp"

#include "fastfall/render/DebugDraw.hpp"

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

Linef travel_form(Linef line, Vec2f body_size)
{
    Vec2f n     = math::normal(line);
    float halfx = body_size.x * 0.5f;
    float ang   = math::angle(line).radians();

    if (!math::is_vertical(line)) {
        if (n.y < 0) {
            // floor
            line.p1.x -= halfx;
            line.p2.x += halfx;
            if (!math::is_horizontal(line)) {
                line.p1.y -= tanf(ang) * halfx;
                line.p2.y += tanf(ang) * halfx;
            }
        }
        else {
            // ceil
            line = math::shift(line, Vec2f{0.f, body_size.y});
            line.p2.x -= halfx;
            line.p1.x += halfx;
            if (!math::is_horizontal(line)) {
                line.p2.y -= tanf(ang) * halfx;
                line.p1.y += tanf(ang) * halfx;
            }
        }
    }
    else {
        line = math::shift(line, { n.x * halfx, 0 });
        if (n.x < 0) {
            // west wall
            line.p1.y += body_size.y;
        }
        else {
            // east wall
            line.p2.y += body_size.y;
        }
    }
    return line;
}

SurfaceFollow::SurfaceFollow(Linef init_path, Vec2f init_pos, float travel_dir, float distance, AngleRange angle_ranges, Angle max_angle, Vec2f collidable_size)
    : curr_path     { 0, init_path, travel_form(init_path, collidable_size), init_pos}
    , travel_dir    { travel_dir }
    , travel_dist   { distance }
    , angle_range   { angle_ranges }
    , angle_max     { max_angle }
    , body_size     { collidable_size }
{
    path_taken.push_back(curr_path);

    if (debug::enabled(debug::Collision_Follow)) {
        auto lines = debug::draw(Primitive::TRIANGLES, 6, {}, {.color = Color::White});
        float depth = 2.f;
        lines[0].pos = curr_path.surface_line.p1;
        lines[1].pos = curr_path.surface_line.p2;
        lines[2].pos = curr_path.surface_line.p1 - math::normal(curr_path.surface_line) * depth;
        lines[3].pos = curr_path.surface_line.p1 - math::normal(curr_path.surface_line) * depth;
        lines[4].pos = curr_path.surface_line.p2;
        lines[5].pos = curr_path.surface_line.p2 - math::normal(curr_path.surface_line) * depth;

        auto travel_lines = debug::draw(Primitive::LINES, 10, {}, { .color = Color::Blue });
        travel_lines[0].pos = curr_path.travel_line.p1;
        travel_lines[1].pos = curr_path.travel_line.p2;
        travel_lines[2].pos = curr_path.start_pos + Vec2f{-1, -1};
        travel_lines[3].pos = curr_path.start_pos + Vec2f{1, -1};
        travel_lines[4].pos = curr_path.start_pos + Vec2f{1, -1};
        travel_lines[5].pos = curr_path.start_pos + Vec2f{1, 1};
        travel_lines[6].pos = curr_path.start_pos + Vec2f{1, 1};
        travel_lines[7].pos = curr_path.start_pos + Vec2f{-1, 1};
        travel_lines[8].pos = curr_path.start_pos + Vec2f{-1, 1};
        travel_lines[9].pos = curr_path.start_pos + Vec2f{-1, -1};
    }
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
        // skip if surface intersect is behind us
        float dir_dot = math::dot(curr_dir, candidate.start_pos - curr_path.start_pos);
        if (dir_dot <= 0) {
            continue;
        }
        // pick surface if it's been than current pick
        if (!curr_pick || compare_paths(travel_dir, curr_path, *curr_pick, candidate)) {
            curr_pick = &candidate;
        }
    }

    if (curr_pick) {
        if (debug::enabled(debug::Collision_Follow)) {
            auto lines = debug::draw(Primitive::TRIANGLES, 6, {}, {.color = Color::Green});
            float depth = 2.f;
            lines[0].pos = curr_pick->surface_line.p1;
            lines[1].pos = curr_pick->surface_line.p2;
            lines[2].pos = curr_pick->surface_line.p1 - math::normal(curr_pick->surface_line) * depth;
            lines[3].pos = curr_pick->surface_line.p1 - math::normal(curr_pick->surface_line) * depth;
            lines[4].pos = curr_pick->surface_line.p2;
            lines[5].pos = curr_pick->surface_line.p2 - math::normal(curr_pick->surface_line) * depth;
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
        .travel_dir     = travel_dir,
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
        .travel_dir     = travel_dir,
        .on_new_surface = false
    };
}

enum class surface_type {
    Floor,
    Wall,
    Ceil
};


surface_type get_surface_type(const Linef& line) {
    if (!math::is_vertical(line)) {
        Vec2f n = math::normal(line);
        if (n.y < 0) {
            return surface_type::Floor;
        }
        else {
            return surface_type::Ceil;
        }
    }
    else {
        return surface_type::Wall;
    }
}

std::optional<SurfaceFollow::surface_path>
SurfaceFollow::valid_surface(Linef path, surface_id id) const
{
    if (path == curr_path.surface_line) {
        // this is the surface we're currently on
        return {};
    }

    Linef travel_line = travel_form(path, body_size);

    // reverse lines if travel_dir < 0.f

    auto dir_curr = curr_path.travel_line;
    auto dir_path = travel_line;

    Vec2f inter = dir_curr.p2 == dir_path.p1
                  ? dir_curr.p2
                  : math::intersection(dir_path, dir_curr);

    surface_type curr_type = get_surface_type(curr_path.travel_line);
    surface_type path_type = get_surface_type(travel_line);

    // apply curr line transform depending on the proposed path
    // constexpr static std::string_view type_names[] = { "floor", "wall", "ceil" };
    // LOG_INFO("{} to {}", type_names[static_cast<int>(curr_type)], type_names[static_cast<int>(path_type)]);

    dir_curr = travel_dir > 0.f ? dir_curr : dir_curr.reverse();
    dir_path = travel_dir > 0.f ? dir_path : dir_path.reverse();

    Vec2f ahead_pos = travel_dir > 0.f ? curr_path.surface_line.p1 : curr_path.surface_line.p2;


    std::optional<Vec2f> intersect;
    if (!std::isnan(inter.x) && !std::isnan(inter.y))
    {
        bool curr_has_inter = math::line_has_point(dir_curr, inter);
        bool path_has_inter = math::line_has_point(dir_path, inter);
        float dot = math::dot(inter - ahead_pos, math::tangent(dir_curr));
        bool is_ahead = dot > 0;

        // make sure the intersection lies on both lines
        if (is_ahead && (curr_has_inter || path_has_inter))
        {
            /*
            // surface connects at the end of curr_path
            bool connect_at_end = inter == dir_curr.p2;

            // surface forms a ramp on curr_path
            auto dir_path_ang = math::angle(dir_path);
            auto dir_curr_ang = math::angle(dir_curr);
            auto diff         = dir_path_ang - dir_curr_ang;
            bool connect_in_middle = diff < 0.f;

            if (connect_at_end || connect_in_middle) {
                intersect = inter;
            }
            */
            intersect = inter;
        }
    }
    else
    {
        bool collinear = math::collinear(path, curr_path.surface_line);
        if (collinear) {
            /*
            bool is_ahead = math::dot(dir_path.p1 - dir_curr.p1, math::tangent(dir_curr)) > 0;
            */

            Vec2f collinear_intersect = (travel_dir > 0.f ? path.p1 : path.p2);
            if (path_type == surface_type::Ceil) {
                collinear_intersect += Vec2f{0, body_size.y};
            }
            else if (path_type == surface_type::Wall) {
                collinear_intersect += math::normal(path) * body_size.x * 0.5f;
            }
            bool has_point = math::line_has_point(dir_curr, collinear_intersect);

            float dot = math::dot(collinear_intersect - ahead_pos, math::tangent(dir_curr));
            bool is_ahead = dot > 0;

            if (is_ahead && has_point) {
                intersect = collinear_intersect;
            }
        }
    }

    Angle next_ang  = math::angle(math::tangent(dir_path));
    Angle curr_ang  = math::angle(math::tangent(dir_curr));

    Angle diff      = next_ang - curr_ang;
    Angle normal    = math::angle(math::normal(path));

    bool in_range   = angle_range.within_range(normal);
    bool in_max_ang = abs(diff.degrees()) < abs(angle_max.degrees());

    bool is_valid = intersect && in_range && in_max_ang;

    if (debug::enabled(debug::Collision_Follow)) {
        auto lines = debug::draw(Primitive::TRIANGLES, 6, {}, { .color = (is_valid ? Color::Green().green(128) : Color::Red) });
        float depth = is_valid ? 2.f : 0.5f;
        lines[0].pos = path.p1;
        lines[1].pos = path.p2;
        lines[2].pos = path.p1 - math::normal(path) * depth;
        lines[3].pos = path.p1 - math::normal(path) * depth;
        lines[4].pos = path.p2;
        lines[5].pos = path.p2 - math::normal(path) * depth;

        if (intersect) {
            auto travel_lines = debug::draw(Primitive::LINES, 10, {}, { .color = Color::Blue });
            travel_lines[0].pos = travel_line.p1;
            travel_lines[1].pos = travel_line.p2;

            auto pos = *intersect;
            travel_lines[2].pos = pos + Vec2f{-1, -1};
            travel_lines[3].pos = pos + Vec2f{1, -1};
            travel_lines[4].pos = pos + Vec2f{1, -1};
            travel_lines[5].pos = pos + Vec2f{1, 1};
            travel_lines[6].pos = pos + Vec2f{1, 1};
            travel_lines[7].pos = pos + Vec2f{-1, 1};
            travel_lines[8].pos = pos + Vec2f{-1, 1};
            travel_lines[9].pos = pos + Vec2f{-1, -1};
        }
    }

    if (is_valid) {
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
