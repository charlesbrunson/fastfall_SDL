
#include "gtest/gtest.h"

#include "fastfall/util/math.hpp"
#include "fastfall/game/phys/CollisionSolver.hpp"

using namespace ff;


TEST(collisionsolver, ghost_edge) {

	{
		Contact c1{
			.separation = 1.f,
			.hasContact = true,
			.ortho_n = {0.f, 1.f},
			.collider_n = {0.f, 1.f},
			.collider = ColliderSurface{
				.surface = {
					{  0, 0 },
					{ 16, 0 }
				}
			}
		};
		Contact c2{
			.separation = 1.f,
			.hasContact = true,
			.ortho_n = { 1.f, 0.f },
			.collider_n = { 1.f, 0.f },
			.collider = ColliderSurface{
				.surface = {
					{  0,  0 },
					{  0, 16 }
				}
			}
		};

		CollisionSolver::Ghost ghost = CollisionSolver::isGhostEdge(c1, c2);
		EXPECT_EQ(ghost, CollisionSolver::Ghost::FULL_GHOST);
	}

	{
		// see wall_to_ceil_clip
		Contact c1{
			.separation	= 1.f,
			.hasContact	= true,
			.ortho_n	= {0.f, 1.f},
			.collider_n = {0.f, 1.f},
			.collider = ColliderSurface{
				.surface = { 
					{ 32, 32 }, 
					{ 16, 32 } 
				}
			}
		};
		Contact c2{
			.separation = 1.f,
			.hasContact = true,
			.ortho_n    = { 1.f, 0.f },
			.collider_n = { 1.f, 0.f },
			.collider = ColliderSurface{
				.surface = {
					{ 32,  0 },
					{ 32, 16 }
				}
			}
		};

		CollisionSolver::Ghost ghost = CollisionSolver::isGhostEdge(c1, c2);
		EXPECT_EQ(ghost, CollisionSolver::Ghost::FULL_GHOST);
	}
}

TEST(collisionsolver, wedge_velocity) {

	Vec2f n1;
	Vec2f n2;
	Vec2f v1;
	Vec2f v2;
	Vec2f result;

	float err = 0.0001;

	// should return Vec2f{ 0, 0 } if normals face each other (not a wedge)
	{
		n1 = Vec2f{ 0, 1 };
		n2 = Vec2f{ 0, -1 };
		v1 = Vec2f{ 0, 0 };
		v2 = Vec2f{ 0, 0 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_EQ(result.x, 0.f);
		EXPECT_EQ(result.y, 0.f);
	}

	// 45 degree slope above flat ground (open +x)
	{
		n1 = Vec2f{ 1, 1 }.unit();
		n2 = Vec2f{ 0, -1 };
		v1 = Vec2f{ 0, 0 };
		v2 = Vec2f{ 0, 0 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, 0.f, err);
		EXPECT_NEAR(result.y, 0.f, err);

		v1 = Vec2f{ 0, 50 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, 50.f, err);
		EXPECT_NEAR(result.y, 0.f, err);

		v1 = Vec2f{ 0, 100 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, 100.f, err);
		EXPECT_NEAR(result.y, 0.f, err);

		v1 = Vec2f{ 0, 0 };
		v2 = Vec2f{ 0, -100 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, 100.f, err);
		EXPECT_NEAR(result.y, 0.f, err);
	}

	// 45 degree slope above flat ground (open -x)
	{
		n1 = Vec2f{ -1, 1 }.unit();
		n2 = Vec2f{ 0, -1 };
		v1 = Vec2f{ 0, 0 };
		v2 = Vec2f{ 0, 0 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, 0.f, err);
		EXPECT_NEAR(result.y, 0.f, err);

		v1 = Vec2f{ 0, 50 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, -50.f, err);
		EXPECT_NEAR(result.y, 0.f, err);

		v1 = Vec2f{ 0, 100 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, -100.f, err);
		EXPECT_NEAR(result.y, 0.f, err);

		v1 = Vec2f{ 0, 0 };
		v2 = Vec2f{ 0, -100 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, -100.f, err);
		EXPECT_NEAR(result.y, 0.f, err);
	}

	// 22.5 degree slope above flat ground (open +x)
	{
		n1 = Vec2f{ 1, 2 }.unit();
		n2 = Vec2f{ 0, -1 };
		v1 = Vec2f{ 0, 0 };
		v2 = Vec2f{ 0, 0 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, 0.f, err);
		EXPECT_NEAR(result.y, 0.f, err);

		v1 = Vec2f{ 0, 50 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, 100.f, err);
		EXPECT_NEAR(result.y, 0.f, err);

		v1 = Vec2f{ 0, 100 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, 200.f, err);
		EXPECT_NEAR(result.y, 0.f, err);

		v1 = Vec2f{ 0, 0 };
		v2 = Vec2f{ 0, -100 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, 200.f, err);
		EXPECT_NEAR(result.y, 0.f, err);
	}

	// 22.5 degree slope above flat ground (open -x)
	{
		n1 = Vec2f{ -1, 2 }.unit();
		n2 = Vec2f{ 0, -1 };
		v1 = Vec2f{ 0, 0 };
		v2 = Vec2f{ 0, 0 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, 0.f, err);
		EXPECT_NEAR(result.y, 0.f, err);

		v1 = Vec2f{ 0, 50 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, -100.f, err);
		EXPECT_NEAR(result.y, 0.f, err);

		v1 = Vec2f{ 0, 100 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, -200.f, err);
		EXPECT_NEAR(result.y, 0.f, err);

		v1 = Vec2f{ 0, 0 };
		v2 = Vec2f{ 0, -100 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, -200.f, err);
		EXPECT_NEAR(result.y, 0.f, err);
	}




	// 45 degree slope (open +x) above 22.5 sloped ground (open -x)
	{
		n1 = Vec2f{ -16, 16 }.lefthand().unit();
		n2 = Vec2f{ 16, -8 }.lefthand().unit();
		v1 = Vec2f{ 0, 50 };
		v2 = Vec2f{ 0, 0 };
		result = CollisionSolver::calcWedgeVel(n1, n2, v1, v2);
		EXPECT_NEAR(result.x, 200.f, err);
		EXPECT_NEAR(result.y, 0.f, err);
	}

}
