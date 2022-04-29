
#include "gtest/gtest.h"

#include "fastfall/util/math.hpp"
#include "fastfall/game/phys/CollisionSolver.hpp"

using namespace ff;

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
}
