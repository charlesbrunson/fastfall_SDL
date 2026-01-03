
#include <numeric>

#include "gtest/gtest.h"
#include "fastfall/game/phys/collidable/SurfaceFollow.hpp"

using namespace ff;

bool add_path(SurfaceFollow& follow, Linef line) {
    return follow.try_push_back(line);
}

TEST(pathfollow, basic)
{
    auto init_line = Linef{
        {0.f,  0.f},
        {16.f, 0.f}
    };

    auto init_pos  = Vec2f{ 8.f, 0.f };

    auto angle_range = AngleRange::Any;

    {
        SurfaceFollow follow(init_line, init_pos, 1.f, 32.f, angle_range, Angle::Degree(180), {});
        {
            // valid
            EXPECT_EQ(add_path(follow, {{10.f, 0.f},
                                        {26.f, -16.f}}), true);

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
            EXPECT_EQ(add_path(follow, {{10.f, 0.f},
                                        {26.f, -16.f}}), false);
            // line is under current path
            EXPECT_EQ(add_path(follow, {{12.f, 2.f},
                                        {14.f, 2.f}}), false);
            // valid
            EXPECT_EQ(add_path(follow, {{26.f, -16.f},
                                        {32.f, -16.f}}), true);

            auto pick_id = follow.pick_surface_to_follow();
            ASSERT_TRUE(pick_id.has_value());

            auto result = follow.travel_to(*pick_id);
            EXPECT_EQ(result.pos.x, 26.f);
            EXPECT_EQ(result.pos.y, -16.f);
            EXPECT_NE(result.dist, 0.f);
            EXPECT_TRUE(result.on_new_surface);
        }
    }
    {
        SurfaceFollow follow(init_line, init_pos, 1.f, 32.f, angle_range, Angle::Degree(180), {});
        EXPECT_EQ(add_path(follow, {{16.f, 0.f},
                                    {32.f, 0.f}}), true);

        auto pick_id = follow.pick_surface_to_follow();
        ASSERT_TRUE(pick_id.has_value());

        auto result = follow.travel_to(*pick_id);
        EXPECT_EQ(result.pos.x, 16.f);
        EXPECT_EQ(result.pos.y, 0.f);
        EXPECT_EQ(result.dist,  24.f);
        EXPECT_TRUE(result.on_new_surface);


        EXPECT_EQ(add_path(follow, {{16.f, 0.f},
                                    {32.f, 0.f}}), false);

        result = follow.finish();
        EXPECT_EQ(result.pos.x, 40.f);
        EXPECT_EQ(result.pos.y, 0.f);
        EXPECT_EQ(result.dist,  0.f);
        EXPECT_FALSE(result.on_new_surface);
    }
}

TEST(pathfollow, wall)
{

    auto init_line = Linef{
            {0.f,  0.f},
            {0.f, 16.f}
    };

    auto init_pos  = Vec2f{ 0.f, 14.f };
    SurfaceFollow follow(init_line, init_pos, 1.f, 32.f, AngleRange::Any, Angle::Degree(180), {8, 8});

    EXPECT_EQ(add_path(follow, {{16.f, -16.f},
                                {0.f,    0.f}}), false);
}

/*
 *    ^|
 *    ^|
 * >>>^|
 * -----
 */

TEST(pathfollow, floor_up_wall)
{
     auto angle_range = AngleRange::Any;

    auto init_line = Linef{ {0.f,  0.f}, {32.f, 0.f} };
    auto next_line = Linef{ init_line.p2, init_line.p2 + Vec2f{ 0.f, -32.f} };
    auto init_pos  = init_line.p1;

    auto follow_max_ang = Angle::Degree(180);
    auto body_size = Vec2f{ 16.f, 16.f};

    float move_dir = 1.f;
    float dist_to_move = 64.f;

    SurfaceFollow follow(init_line, init_pos, move_dir, dist_to_move, angle_range, follow_max_ang, body_size);
    EXPECT_EQ(add_path(follow, next_line), true);

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
 * ----v
 *    |v
 *    |v
 */

TEST(pathfollow, floor_down_wall)
{
    auto angle_range = AngleRange::Any;

    auto init_line = Linef{ {0.f,  0.f}, {32.f, 0.f} };
    auto next_line = Linef{ init_line.p2, init_line.p2 + Vec2f{ 0.f, 32.f} };
    auto init_pos  = init_line.p1;

    auto follow_max_ang = Angle::Degree(180);
    auto body_size = Vec2f{ 16.f, 16.f};

    float move_dir = 1.f;
    float dist_to_move = 64.f;

    SurfaceFollow follow(init_line, init_pos, move_dir, dist_to_move, angle_range, follow_max_ang, body_size);
    EXPECT_EQ(add_path(follow, next_line), true);

    /*
    auto pick_id = follow.pick_surface_to_follow();
    ASSERT_TRUE(pick_id.has_value());

    auto result = follow.travel_to(*pick_id);
    EXPECT_EQ(result.pos.x, 40);
    EXPECT_EQ(result.pos.y, next_line.p1.y);
    EXPECT_EQ(result.dist, 24.f);
    EXPECT_TRUE(result.on_new_surface);
    */
}

/*
 * -----
 * >>>v|
 *    v|
 *    v|
 */

TEST(pathfollow, ceil_down_wall)
{
    auto angle_range = AngleRange::Any;

    auto init_line = Linef{ {32.f,  0.f}, { 0.f, 0.f} };
    auto next_line = Linef{ {32.f, 32.f}, {32.f, 0.f} };
    auto init_pos  = init_line.p1;

    auto follow_max_ang = Angle::Degree(180);
    auto body_size = Vec2f{ 16.f, 16.f};

    float move_dir = 1.f;
    float dist_to_move = 64.f;

    SurfaceFollow follow(init_line, init_pos, move_dir, dist_to_move, angle_range, follow_max_ang, body_size);
    EXPECT_EQ(add_path(follow, next_line), true);

    /*
    auto pick_id = follow.pick_surface_to_follow();
    ASSERT_TRUE(pick_id.has_value());

    auto result = follow.travel_to(*pick_id);
    EXPECT_EQ(result.pos.x, 40);
    EXPECT_EQ(result.pos.y, next_line.p1.y);
    EXPECT_EQ(result.dist, 24.f);
    EXPECT_TRUE(result.on_new_surface);
    */
}

/*
 *    |^
 *    |^
 * ----^
 * >>>>^
 */

TEST(pathfollow, ceil_up_wall)
{

    auto angle_range = AngleRange::Any;

    auto init_line = Linef{ {0.f,  32.f}, {32.f, 32.f} };
    auto next_line = Linef{ init_line.p2, init_line.p2 + Vec2f{ 0.f, -32.f} };
    auto init_pos  = init_line.p1;

    auto follow_max_ang = Angle::Degree(180);
    auto body_size = Vec2f{ 16.f, 16.f};

    float move_dir = 1.f;
    float dist_to_move = 64.f;

    SurfaceFollow follow(init_line, init_pos, move_dir, dist_to_move, angle_range, follow_max_ang, body_size);
    EXPECT_EQ(add_path(follow, next_line), true);
}

/*
 * >>>>>
 * ^----
 * ^|
 * ^|
 */

TEST(pathfollow, wall_up_floor)
{
    auto angle_range = AngleRange::Any;

    auto init_line = Linef{ {0.f,  32.f}, {0.f, 0.f} };
    auto next_line = Linef{ init_line.p2, init_line.p2 + Vec2f{ 32.f, 0.f} };
    auto init_pos  = init_line.p1;

    auto follow_max_ang = Angle::Degree(180);
    auto body_size = Vec2f{ 16.f, 16.f};

    float move_dir = 1.f;
    float dist_to_move = 64.f;

    SurfaceFollow follow(init_line, init_pos, move_dir, dist_to_move, angle_range, follow_max_ang, body_size);
    EXPECT_EQ(add_path(follow, next_line), true);
}

/*
 * -----
 * <<<^|
 *    ^|
 *    ^|
 */

TEST(pathfollow, wall_up_ceil)
{
    auto angle_range = AngleRange::Any;

    auto init_line = Linef{ {32.f,  32.f}, {32.f, 0.f} };
    auto next_line = Linef{ init_line.p2, init_line.p2 + Vec2f{ -32.f, 0.f} };
    auto init_pos  = init_line.p1;

    auto follow_max_ang = Angle::Degree(180);
    auto body_size = Vec2f{ 16.f, 16.f};

    float move_dir = 1.f;
    float dist_to_move = 64.f;

    SurfaceFollow follow(init_line, init_pos, move_dir, dist_to_move, angle_range, follow_max_ang, body_size);
    EXPECT_EQ(add_path(follow, next_line), true);
}

/*
 *    |v
 *    |v
 * ----v
 * <<<<<
 */

TEST(pathfollow, wall_down_ceil)
{

    auto angle_range = AngleRange::Any;

    auto init_line = Linef{ {32.f,  0.f}, {32.f, 32.f} };
    auto next_line = Linef{ init_line.p2, init_line.p2 + Vec2f{ -32.f, 0.f} };
    auto init_pos  = init_line.p1;

    auto follow_max_ang = Angle::Degree(180);
    auto body_size = Vec2f{ 16.f, 16.f};

    float move_dir = 1.f;
    float dist_to_move = 64.f;

    SurfaceFollow follow(init_line, init_pos, move_dir, dist_to_move, angle_range, follow_max_ang, body_size);
    EXPECT_EQ(add_path(follow, next_line), true);
}

/*
 * |v
 * |v
 * |>>>>
 * -----
 */

TEST(pathfollow, wall_down_floor)
{

    auto angle_range = AngleRange::Any;

    auto init_line = Linef{ {0.f,  0.f}, {0.f, 32.f} };
    auto next_line = Linef{ init_line.p2, init_line.p2 + Vec2f{ 32.f, 0.f} };
    auto init_pos  = init_line.p1;

    auto follow_max_ang = Angle::Degree(180);
    auto body_size = Vec2f{ 16.f, 16.f};

    float move_dir = 1.f;
    float dist_to_move = 64.f;

    SurfaceFollow follow(init_line, init_pos, move_dir, dist_to_move, angle_range, follow_max_ang, body_size);
    EXPECT_EQ(add_path(follow, next_line), true);
}

TEST(pathfollow, intersect_test1)
{

    auto angle_range = AngleRange::Any;

    auto init_line = Linef{ {0.f,  0.f}, {16.f, 0.f} };
    auto init_pos  = init_line.p1;

    auto follow_max_ang = Angle::Degree(90);
    auto body_size = Vec2f{ 16.f, 16.f};

    float move_dir = 1.f;
    float dist_to_move = 32.f;

    struct candidate_t
    {
        Linef line;
        bool should_add;
    };

    std::vector<candidate_t> candidates = {
        { .line = { {-16, 16 }, {0, 8} }, .should_add = false},
        { .line = { {16, 0}, {32, 0} }, .should_add = true},
    };

    SurfaceFollow follow(init_line, init_pos, move_dir, dist_to_move, angle_range, follow_max_ang, body_size);

    int total_candidates = std::count_if(candidates.begin(), candidates.end(), [](const auto& cmp)
    {
        return cmp.should_add;
    });
    for (const auto& cand : candidates)
    {
        bool added = add_path(follow, cand.line);
        EXPECT_EQ(added, cand.should_add);
    }

    EXPECT_EQ(follow.get_path_candidates().size(), total_candidates);

    auto result =  follow.pick_surface_to_follow();
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value(), 0);
}
