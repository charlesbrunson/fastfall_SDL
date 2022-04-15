#include "gtest/gtest.h"

#include "fastfall/game/Instance.hpp"
#include "fastfall/game/object/ObjectComponents.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderTileMap.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/collidable/SurfaceTracker.hpp"

using namespace ff;


class surfacetracker_slope : public ::testing::Test {

protected:
	CollisionManager colMan;

	Collidable* box;
	SurfaceTracker* ground;

	ColliderTileMap* collider;

	surfacetracker_slope()
		: colMan{0u}
	{
	}

	virtual ~surfacetracker_slope() {
	}

	void SetUp() override {
		
		collider = colMan.create_collider<ColliderTileMap>(Vec2i{ 5, 5 });

		Vec2f pos  = { 8, 32 };
		Vec2f size = { 16, 32 };
		Vec2f grav = { 0, 100 };
		box = colMan.create_collidable(pos, size, grav);

		ground = &box->create_tracker(
			Angle::Degree(-135),
			Angle::Degree(-45),
			SurfaceTracker::Settings{
				.slope_sticking = true,
				.stick_angle_max = Angle::Degree(91)
			});
	}

	void TearDown() override {

	}

};



void setTiles(ColliderTileMap* map, grid_vector<std::string_view> tiles) {

	for (auto it = tiles.begin(); it != tiles.end(); it++) {
		if (!it->empty()) {
			map->setTile({ (int)it.column(), (int)it.row() }, TileShape::from_string(*it));
		}
	}
	map->applyChanges();
}




TEST_F(surfacetracker_slope, stick_slope_down)
{
	constexpr float one_frame = (1.f / 60.f);

	setTiles(collider, {
		/* y:0 _*/ {"",			"",			"",			"",			""},
		/* y:16_*/ {"",			"",			"",			"",			""},		
		/* y:32_*/ {"solid",	"solid",	"slope-h",	"",			""},
		/* y:48_*/ {"solid",	"solid",	"solid",	"slope-h",	""},		
		/* y:64_*/ {"solid",	"solid",	"solid",	"solid",	"slope-h"}, 
	});

	box->set_vel(Vec2f{ 50.f, 0.f });

	for (int i = 0; i < 300 && box->getPosition().x <= 80.f && box->getPosition().x >= 0.f; i++) {
		collider->update(one_frame);
		box->update(one_frame);
		colMan.update(one_frame);

		//fmt::print(stderr, "{} - {:2.1f} has_contact() = {}\n", i, box->getPosition(), ground->has_contact());
		EXPECT_TRUE(ground->has_contact());
	}
}