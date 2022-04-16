#include "gtest/gtest.h"

#include "fastfall/game/Instance.hpp"
#include "fastfall/game/object/ObjectComponents.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderTileMap.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/collidable/SurfaceTracker.hpp"

#include "TestPhysRenderer.hpp"

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

		Vec2f pos  = { 0, 0 };
		Vec2f size = { 16, 32 };
		Vec2f grav = { 0, 400 };
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

	void update(secs duration) {
		collider->update(duration);
		box->update(duration);
		colMan.update(duration);
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
	// Goal: Surface tracker should stick to slopes when slope_sticking is set.
	// Action: slide down a slope and don't lose contact on any frame

	std::string test_name = fmt::format("{}__{}",
		::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name(),
		::testing::UnitTest::GetInstance()->current_test_info()->name());

	constexpr secs one_frame = (1.0 / 60.0);

	setTiles(collider, {
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"",			"",			"",			"",			""},
		/* y:16_*/ {"",			"",			"",			"",			""},		
		/* y:32_*/ {"solid",	"solid",	"slope-h",	"",			""},
		/* y:48_*/ {"solid",	"solid",	"solid",	"slope-h",	""},		
		/* y:64_*/ {"solid",	"solid",	"solid",	"solid",	"slope-h"}, 
	});

	box->teleport({ 8, 32 });
	box->set_vel(Vec2f{ 100.f, 0.f });

	TestPhysRenderer render({ 0, 0, 80, 80 });

	for (int i = 0; i < 300 && box->getPosition().x <= 80.f && box->getPosition().x >= 0.f; i++) 
	{
		update(one_frame);
		render.render(colMan, test_name, i);
		EXPECT_TRUE(ground->has_contact());
	}
}

TEST_F(surfacetracker_slope, stick_touchngo)
{
	// Goal:   Surface tracker should stick to slopes on the very frame we collide.
	// Action: fall onto ground just before, keep sliding without losing contact

	std::string test_name = fmt::format("{}__{}",
		::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name(),
		::testing::UnitTest::GetInstance()->current_test_info()->name());

	constexpr secs one_frame = (1.0 / 60.0);

	setTiles(collider, {
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"",			"",			"",			"",			""},
		/* y:16_*/ {"",			"",			"",			"",			""},
		/* y:32_*/ {"solid",	"solid",	"slope-h",	"",			""},
		/* y:48_*/ {"solid",	"solid",	"solid",	"slope-h",	""},
		/* y:64_*/ {"solid",	"solid",	"solid",	"solid",	"slope-h"},
		});

	box->teleport({ 32 - 5, 31 });
	box->set_vel(Vec2f{ 6.f, 6.f } / one_frame);

	fmt::print(stderr, "frame 0. box pos = {}\n", box->getPosition());
	TestPhysRenderer render({0, 0, 80, 80});

	render.render(colMan, test_name, 0);

	update(one_frame);
	fmt::print(stderr, "frame 1. box pos = {}\n", box->getPosition());
	render.render(colMan, test_name, 1);
	EXPECT_TRUE(ground->has_contact());

	update(one_frame);
	fmt::print(stderr, "frame 2. box pos = {}\n", box->getPosition());
	render.render(colMan, test_name, 2);
	EXPECT_TRUE(ground->has_contact());

	for (int i = 3; i < 300 && box->getPosition().x <= 80.f && box->getPosition().x >= 0.f; i++)
	{
		update(one_frame);
		render.render(colMan, test_name, i);
	}

}
