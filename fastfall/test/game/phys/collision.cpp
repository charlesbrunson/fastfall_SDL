#include "gtest/gtest.h"

#include "fastfall/game/Instance.hpp"
#include "fastfall/game/object/ObjectComponents.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderTileMap.hpp"
#include "fastfall/game/phys/Collidable.hpp"

using namespace ff;

TEST(collision, ghostcheck_slopedceil_to_wall)
{
	ColliderTileMap collider{ Vec2i{ 2, 2 } };
	collider.setTile({ 0, 0 }, { "solid" });
	collider.setTile({ 0, 1 }, { "slope-hv" });
	collider.applyChanges();

	Collidable collidable{ Vec2f{ 18, 48 }, Vec2f{ 16, 32 } };
	collidable.move(Vec2f{ 0.f, -8.f });
	collidable.set_vel(Vec2f{ 0.f, -100.f });

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
	EXPECT_TRUE((contact.ortho_normal == Vec2f{ 0.f, 1.f }));

}