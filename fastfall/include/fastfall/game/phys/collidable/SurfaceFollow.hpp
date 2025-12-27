#pragma once

#include <vector>

#include "fastfall/util/math.hpp"

namespace ff {

    class SurfaceFollow {
    public:

        struct Path {
            size_t index = {};
            Linef  surface_line = {};
            Linef  travel_line = {};
            Vec2f  start_pos = {};
            Angle  angle = {};
            Angle  diff_angle = {};
        };

        struct Result {
            Path  path = {};
            float dist = {};
            Vec2f pos  = {};
            float travel_dir = 1.f;
            bool  on_new_surface = false;
        };

        static bool compare_paths(float travel_dir, const Path& from, const Path& pick, const Path& candidate);

        SurfaceFollow(Linef init_path, Vec2f init_pos, float travel_dir, float distance, AngleRange angle_ranges, Angle max_angle, Vec2f collidable_size);

        void reset();

        [[nodiscard]] bool try_push_back(Linef path);

        [[nodiscard]] std::optional<size_t> pick_surface_to_follow() const;
        [[nodiscard]] Result travel_to(size_t candidate_ndx);

        [[nodiscard]] float remaining_distance() const { return travel_dist; }
        [[nodiscard]] explicit operator bool() const { return travel_dist > 0; }
        [[nodiscard]] const Path& current_path() const { return curr_path; }

        [[nodiscard]] const std::vector<Path>& get_path_taken()      const { return path_taken; }
        [[nodiscard]] const std::vector<Path>& get_path_candidates() const { return path_candidates; }

        Result finish();

    private:
        [[nodiscard]] bool add_surface_if_valid(Linef path);

        Path curr_path = {};

        float travel_dir = {};
        float travel_dist = 0.f;

        Vec2f body_size = {};

        Angle angle_max = {};
        AngleRange angle_range = {};

        std::vector<Path> path_candidates;
        std::vector<Path> path_taken;
    };

}