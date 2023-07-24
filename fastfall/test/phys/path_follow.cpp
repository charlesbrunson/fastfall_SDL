
#include "gtest/gtest.h"
#include "fastfall/game/phys/collidable/SurfaceFollow.hpp"

using namespace ff;

void add_path(SurfaceFollow& follow, Linef line, bool expect) {
    auto add_id = follow.add_surface(line);
    EXPECT_EQ(add_id.has_value(), expect);
}

TEST(pathfollow, basic) {

    Linef init_line{{0.f,  0.f},
                    {16.f, 0.f}};
    Vec2f init_pos = {8.f, 0.f};

    SurfaceTracker::applicable_ang_t angle_range{
            .min = Angle::Degree(-90) - Angle::Degree(45),
            .max = Angle::Degree(-90) + Angle::Degree(45),
            .inclusive = true
    };

    SurfaceFollow follow(init_line, init_pos, 1.f, 32.f, angle_range, Angle::Degree(90));
    {
        // valid
        add_path(follow, {{10.f, 0.f}, {26.f, -16.f}}, true);

        auto pick_id = follow.pick_surface_to_follow();
        ASSERT_TRUE(pick_id.has_value());

        auto result = follow.travel_to(*pick_id);
        EXPECT_EQ(result.pos.x, 10.f);
        EXPECT_EQ(result.pos.y, 0.f);
        EXPECT_EQ(result.dist, 32.f - 2.f);
    }
    {
        // repeat current path
        add_path(follow, {{10.f, 0.f},   {26.f, -16.f}}, false);
        // line is under current path
        add_path(follow, {{12.f, -2.f},  {14.f, -2.f}}, false);
        // valid
        add_path(follow, {{26.f, -16.f}, {32.f, -16.f}}, true);

        auto pick_id = follow.pick_surface_to_follow();
        ASSERT_TRUE(pick_id.has_value());

        auto result = follow.travel_to(*pick_id);
        EXPECT_EQ(result.pos.x,  26.f);
        EXPECT_EQ(result.pos.y, -16.f);
        EXPECT_NE(result.dist,   0.f);
    }
}