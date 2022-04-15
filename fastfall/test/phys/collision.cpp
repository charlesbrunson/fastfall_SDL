#include "gtest/gtest.h"

#include "fastfall/game/Instance.hpp"
#include "fastfall/game/object/ObjectComponents.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderTileMap.hpp"
#include "fastfall/game/phys/Collidable.hpp"

using namespace ff;

TEST(collision, ghostcheck_slopedceil_to_wall)
{
	constexpr float one_frame = (1.f / 60.f);

	ColliderTileMap collider{ Vec2i{ 5, 5 } };
	collider.setTile({ 0, 0 }, "solid"_ts);
	collider.setTile({ 0, 1 }, "slope-hv"_ts);
	collider.setTile({ 0, 3 }, "solid"_ts);
	collider.setTile({ 1, 3 }, "solid"_ts);
	collider.applyChanges();

	Collidable collidable{ Vec2f{ 20, 48 }, Vec2f{ 16, 32 } };
	collidable.set_vel(Vec2f{ 0.f, -8.f  } / one_frame);
	collidable.update(one_frame);

	RegionArbiter arb{ &collider, &collidable };
	arb.updateRegion(collidable.getBoundingBox());

	CollisionSolver solver{ &collidable };
	for (auto& [quad, arbiter] : arb.getQuadArbiters())
	{
		arbiter.update(1.f / 60.f);
		solver.pushArbiter(&arbiter);
	}
	solver.solve();

	EXPECT_EQ(solver.frame.size(), 1);

	Contact contact = solver.frame[0].contact;
	EXPECT_TRUE(contact.hasContact);
	EXPECT_TRUE(contact.ortho_normal == Vec2f(0.f, 1.f) );
	EXPECT_TRUE(contact.collider_normal == Vec2f(1.f, 1.f).unit() );
	EXPECT_EQ(contact.impactTime, 0.5f);

}