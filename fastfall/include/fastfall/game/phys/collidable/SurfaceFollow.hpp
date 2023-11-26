#pragma once

#include <vector>

#include "fastfall/util/math.hpp"
#include "fastfall/game/phys/collidable/SurfaceTracker.hpp"

namespace ff {

    class SurfaceFollow {
    public:
        using surface_id = size_t;

        struct surface_path {
            surface_id  id;
            Linef       surface_line;
            Linef       travel_line;
            Vec2f       start_pos;
            Angle       angle;
            Angle       diff_angle;
        };

        static bool compare_paths(float travel_dir, const surface_path& from, const surface_path& pick, const surface_path& candidate);

        struct travel_result {
            surface_path path = {};
            float        dist = {};
            Vec2f        pos  = {};
            bool         on_new_surface = false;
        };

        SurfaceFollow(Linef init_path, Vec2f init_pos, float travel_dir, float distance, SurfaceTracker::applicable_ang_t angle_ranges, Angle max_angle, Vec2f collidable_size);

        void reset();

        [[nodiscard]] std::optional<surface_id> add_surface(Linef path);

        [[nodiscard]] std::optional<surface_id> pick_surface_to_follow();
        [[nodiscard]] travel_result travel_to(surface_id id);

        [[nodiscard]] float remaining_distance() const { return travel_dist; }
        [[nodiscard]] explicit operator bool() const { return travel_dist > 0; }
        [[nodiscard]] const surface_path& current_path() const { return curr_path; }

        //[[nodiscard]] const surface_path& operator[] (surface_id id) const { return candidate_paths[id]; }

        [[nodiscard]] const std::vector<surface_path>& get_path_taken()      const { return path_taken; }
        [[nodiscard]] const std::vector<surface_path>& get_path_candidates() const { return candidate_paths; }

        travel_result finish();

    private:

        [[nodiscard]] std::optional<surface_path> valid_surface(Linef path, surface_id id) const;

        surface_path curr_path;

        float travel_dir;
        float travel_dist = 0.f;

        Vec2f body_size = {};

        Angle angle_max;
        SurfaceTracker::applicable_ang_t angle_range;

        std::vector<surface_path> candidate_paths;
        std::vector<surface_path> path_taken;
    };

}