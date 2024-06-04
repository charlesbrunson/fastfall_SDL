

#include "fastfall/game/phys/CollisionSolver.hpp"
#include "gtest/gtest.h"

using namespace ff;

TEST(ghostedge, two_platforms) {
    ContinuousContact c1;
    ContinuousContact c2;

    c1.ortho_n    = Vec2f{ 0.f, -1.f };
    c1.hasContact = true;
    c1.separation = 1.f;
    c1.impactTime = 0.1;
    c1.collider.surface = Linef{ {0.f, 16.f}, {16.f, 0.f} };

    c2.ortho_n    = Vec2f{ 0.f, -1.f };
    c2.hasContact = true;
    c2.separation = 15.f;
    c2.impactTime = 0.1;
    c2.collider.surface  = Linef{ {16.f, 0.f}, {32.f, 0.f} };


    auto ghost1 = isGhostEdge(c1, c2);
    EXPECT_NE(ghost1, GhostEdge::None);

    auto ghost2 = isGhostEdge(c2, c1);
    EXPECT_NE(ghost2, GhostEdge::None);

    auto result = compare(&c1, &c2);

    EXPECT_EQ(ContactType::NO_SOLUTION, result.type);
    EXPECT_EQ(false, result.contact.has_value());

    EXPECT_EQ(false, result.discardFirst);
    EXPECT_EQ(true,  result.discardSecond);

}

TEST(ghostedge, moving_slope_to_oneway) {
    ContinuousContact c1;
    ContinuousContact c2;

    c1.hasContact       = true;
    c1.separation       = 0.13888931274414063;
    c1.collider.surface = Linef{ {54, 32}, {70, 32} };
    c1.ortho_n          = Vec2f{ 0.f, -1.f };
    c1.collider_n       = Vec2f{ 0.f, -1.f };
    c1.hasImpactTime    = false;
    c1.impactTime       = 0.0;
    c1.velocity         = Vec2f{ 179.99998, 0 };
    c1.is_transposed    = false;
    c1.stickOffset      = 0.0;
    c1.stickLine        = Linef{};
    c1.is_solid         = false;

    c2.hasContact       = true;
    c2.separation       = 0.13888931274414063;
    c2.collider.surface = Linef{ {70, 32}, {86, 48} };
    c2.ortho_n          = Vec2f{ 0.f, -1.f };
    c2.collider_n       = Vec2f{ 0.f, -1.f };
    c2.hasImpactTime    = false;
    c2.impactTime       = 0.0;
    c2.velocity         = Vec2f{ 179.99998, 0 };
    c2.is_transposed    = false;
    c2.stickOffset      = 0.0;
    c2.stickLine        = Linef{};
    c2.is_solid         = true;

    auto ghost1 = isGhostEdge(c1, c2);
    EXPECT_NE(ghost1, GhostEdge::None);

    auto ghost2 = isGhostEdge(c2, c1);
    EXPECT_NE(ghost2, GhostEdge::None);

    auto result = compare(&c1, &c2);

    EXPECT_EQ(ContactType::NO_SOLUTION, result.type);
    EXPECT_EQ(false, result.contact.has_value());

    EXPECT_EQ(false, result.discardFirst);
    EXPECT_EQ(false, result.discardSecond);

}

TEST(ghostedge, slopeceil_to_wall) {
    ContinuousContact c1;
    ContinuousContact c2;

    c1.hasContact       = true;
    c1.separation       = 4.f;
    c1.collider.surface = Linef{ {16, 0}, {16, 16} };
    c1.ortho_n          = Vec2f{ 1, 0 };
    c1.collider_n       = Vec2f{ 1, 0 };
    c1.hasImpactTime    = false;
    c1.impactTime       = -1.0;
    c1.velocity         = Vec2f{ 0, 0 };
    c1.is_transposed    = false;
    c1.stickOffset      = 0.0;
    c1.stickLine        = Linef{};

    c2.hasContact       = true;
    c2.separation       = 4.f;
    c2.collider.surface = Linef{ {16, 16}, {0, 32} };
    c2.ortho_n          = Vec2f{ 0, 1 };
    c2.collider_n       = Vec2f{ 0.70710677, 0.70710677 };
    c2.hasImpactTime    = true;
    c2.impactTime       = 0.5;
    c2.velocity         = Vec2f{ 0, 0 };
    c2.is_transposed    = false;
    c2.stickOffset      = 0.0;
    c2.stickLine        = Linef{};

    auto ghost1 = isGhostEdge(c1, c2);
    EXPECT_NE(ghost1, GhostEdge::None);

    auto ghost2 = isGhostEdge(c2, c1);
    EXPECT_NE(ghost2, GhostEdge::None);

    auto result = compare(&c1, &c2);

    EXPECT_EQ(ContactType::NO_SOLUTION, result.type);
    EXPECT_EQ(false, result.contact.has_value());

    EXPECT_EQ(true, result.discardFirst);
    EXPECT_EQ(false, result.discardSecond);

}

TEST(ghostedge, under_oneway) {
    ContinuousContact c1;
    ContinuousContact c2;

    // the oneway
    c1.hasContact       = false;
    c1.separation       = 18.2499542;
    c1.collider.surface = Linef{ {816, 80}, {832, 80} };
    c1.ortho_n          = Vec2f{ 0, -1 };
    c1.collider_n       = Vec2f{ 0, -1 };
    c1.hasImpactTime    = false;
    c1.impactTime       = -1.0;
    c1.velocity         = Vec2f{ 0, 0 };
    c1.is_transposed    = false;
    c1.stickOffset      = 0.0;
    c1.stickLine        = Linef{};
    c1.is_solid         = false;

    c2.hasContact       = true;
    c2.separation       = 3.18054199;
    c2.collider.surface = Linef{ {832, 96}, {832, 80} };
    c2.ortho_n          = Vec2f{ -1, 0 };
    c2.collider_n       = Vec2f{ -1, 0 };
    c2.hasImpactTime    = false;
    c2.impactTime       = 0.5;
    c2.velocity         = Vec2f{ 0, 0 };
    c2.is_transposed    = false;
    c2.stickOffset      = 0.0;
    c2.stickLine        = Linef{};
    c2.is_solid         = true;

    auto ghost1 = isGhostEdge(c1, c2);
    EXPECT_EQ(ghost1, GhostEdge::None);

    auto ghost2 = isGhostEdge(c2, c1);
    EXPECT_EQ(ghost2, GhostEdge::None);

    auto result = compare(&c1, &c2);

    EXPECT_EQ(ContactType::NO_SOLUTION, result.type);
    EXPECT_EQ(false, result.contact.has_value());

    EXPECT_EQ(false, result.discardFirst);
    EXPECT_EQ(false, result.discardSecond);

}

TEST(ghostedge, slope_to_oneway) {
    ContinuousContact c1;
    ContinuousContact c2;

    // the oneway
    c1.hasContact       = false;
    c1.separation       = 0.138885498;
    c1.collider.surface = Linef{ {880, 80}, {896, 80} };
    c1.ortho_n          = Vec2f{ 0, -1 };
    c1.collider_n       = Vec2f{ 0, -1 };
    c1.hasImpactTime    = false;
    c1.impactTime       = -1.0;
    c1.velocity         = Vec2f{ 0, 0 };
    c1.is_transposed    = false;
    c1.stickOffset      = 0.0;
    c1.stickLine        = Linef{};
    c1.is_solid         = false;
    c1.on_center        = true;

    c2.hasContact       = true;
    c2.separation       = 1.42178345;
    c2.collider.surface = Linef{ {864, 88}, {880, 80} };
    c2.ortho_n          = Vec2f{ -1, 0 };
    c2.collider_n       = Vec2f{ -0.44721359, -0.89442718 };
    c2.hasImpactTime    = false;
    c2.impactTime       = 0;
    c2.velocity         = Vec2f{ 0, 0 };
    c2.is_transposed    = false;
    c2.stickOffset      = 0.0;
    c2.stickLine        = Linef{};
    c2.is_solid         = true;
    c2.on_center        = false;

    auto ghost1 = isGhostEdge(c1, c2);
    EXPECT_EQ(ghost1, GhostEdge::Full);

    auto ghost2 = isGhostEdge(c2, c1);
    EXPECT_EQ(ghost2, GhostEdge::Full);

    auto result = compare(&c1, &c2);

    EXPECT_EQ(ContactType::NO_SOLUTION, result.type);
    EXPECT_EQ(false, result.contact.has_value());

    EXPECT_EQ(false, result.discardFirst);
    EXPECT_EQ(true, result.discardSecond);

}