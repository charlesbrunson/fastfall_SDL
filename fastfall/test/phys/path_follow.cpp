
#include "gtest/gtest.h"
#include "fastfall/game/phys/collidable/SurfaceFollow.hpp"

using namespace ff;

void add_path(SurfaceFollow& follow, Linef line, bool expect) {
    auto add_id = follow.add_surface(line);
    EXPECT_EQ(add_id.has_value(), expect);
}

TEST(pathfollow, basic)
{
    auto init_line = Linef{
        {0.f,  0.f},
        {16.f, 0.f}
    };

    auto init_pos  = Vec2f{ 8.f, 0.f };

    auto angle_range = SurfaceTracker::applicable_ang_t{
        .min = Angle::Degree(-90) - Angle::Degree(45),
        .max = Angle::Degree(-90) + Angle::Degree(45),
        .inclusive = true
    };

    SurfaceFollow follow(init_line, init_pos, 1.f, 32.f, angle_range, Angle::Degree(180), {});
    {
        // valid
        add_path(follow, {{10.f, 0.f}, {26.f, -16.f}}, true);

        auto pick_id = follow.pick_surface_to_follow();
        ASSERT_TRUE(pick_id.has_value());

        auto result = follow.travel_to(*pick_id);
        EXPECT_EQ(result.pos.x, 10.f);
        EXPECT_EQ(result.pos.y, 0.f);
        EXPECT_EQ(result.dist, 32.f - 2.f);
        EXPECT_TRUE(result.on_new_surface);
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
        EXPECT_TRUE(result.on_new_surface);
    }
}


/*
 *    ^|
 *    ^|
 * >>>^|
 * -----
 */

TEST(pathfollow, floor_up_wall)
{
     auto angle_range = SurfaceTracker::applicable_ang_t{
            .min = Angle::Degree(std::nextafterf(-180.f, 0.f)),
            .max = Angle::Degree(180.f),
            .inclusive = true
    };

    auto init_line = Linef{ {0.f,  0.f}, {32.f, 0.f} };
    auto next_line = Linef{ init_line.p2, init_line.p2 + Vec2f{ 0.f, -32.f} };
    auto init_pos  = init_line.p1;

    auto follow_max_ang = Angle::Degree(180);
    auto body_size = Vec2f{ 16.f, 16.f};

    float move_dir = 1.f;
    float dist_to_move = 64.f;

    SurfaceFollow follow(init_line, init_pos, move_dir, dist_to_move, angle_range, follow_max_ang, body_size);
    add_path(follow, next_line, true);

    auto pick_id = follow.pick_surface_to_follow();
    ASSERT_TRUE(pick_id.has_value());

    auto result = follow.travel_to(*pick_id);
    EXPECT_EQ(result.pos.x, 24.f);
    EXPECT_EQ(result.pos.y, next_line.p1.y);
    EXPECT_EQ(result.dist, 40.f);
    EXPECT_TRUE(result.on_new_surface);
}

/*
 * >>>>v
 * ---|v
 *    |v
 *    |v
 */

TEST(pathfollow, floor_down_wall)
{
    auto angle_range = SurfaceTracker::applicable_ang_t{
        .min = Angle::Degree(std::nextafterf(-180.f, 0.f)),
        .max = Angle::Degree(180.f),
        .inclusive = true
    };

    auto init_line = Linef{ {0.f,  0.f}, {32.f, 0.f} };
    auto next_line = Linef{ init_line.p2, init_line.p2 + Vec2f{ 0.f, 32.f} };
    auto init_pos  = init_line.p1;

    auto follow_max_ang = Angle::Degree(180);
    auto body_size = Vec2f{ 16.f, 16.f};

    float move_dir = 1.f;
    float dist_to_move = 64.f;

    SurfaceFollow follow(init_line, init_pos, move_dir, dist_to_move, angle_range, follow_max_ang, body_size);
    add_path(follow, next_line, true);

    auto pick_id = follow.pick_surface_to_follow();
    ASSERT_TRUE(pick_id.has_value());

    auto result = follow.travel_to(*pick_id);
    EXPECT_EQ(result.pos.x, 40);
    EXPECT_EQ(result.pos.y, next_line.p1.y);
    EXPECT_EQ(result.dist, 24.f);
    EXPECT_TRUE(result.on_new_surface);
}

/*
 * ---|
 * <<<|
 *   ^|
 *   ^|
 */

TEST(pathfollow, wall_up_ceil)
{

}


/*
 *    |v
 *    |v
 * ---|v
 * <<<<<
 */

TEST(pathfollow, wall_down_ceil)
{

}