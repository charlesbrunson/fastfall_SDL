#include "gtest/gtest.h"

#include "fastfall/game/Instance.hpp"
#include "fastfall/game/object/ObjectComponents.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderTileMap.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/collidable/SurfaceTracker.hpp"

#include "TestPhysRenderer.hpp"

using namespace ff;

class surfacetracker : public ::testing::Test {

protected:
	CollisionManager colMan;

	Collidable* box;
	SurfaceTracker* ground;

	ColliderTileMap* collider = nullptr;

	static constexpr secs one_frame = (1.0 / 60.0);

	surfacetracker()
		: colMan{0u}
	{
	}

	virtual ~surfacetracker() {
	}

	void SetUp() override {
		

		Vec2f pos  = { 0, 0 };
		Vec2f size = { 16, 32 };
		Vec2f grav = { 0, 0 };
		box = colMan.create_collidable(pos, size, grav);

		ground = &box->create_tracker(
			Angle::Degree(-135),
			Angle::Degree(-45),
			SurfaceTracker::Settings{
				.slope_sticking = true,
				.stick_angle_max = Angle::Degree(91)
			});
	}

	void TearDown() override 
	{
	}

	void update() 
	{
		if (collider) {
			collider->update(one_frame);
		}
		box->update(one_frame);
		colMan.update(one_frame);
	}

	void initTileMap(grid_vector<std::string_view> tiles) 
	{
		collider = colMan.create_collider<ColliderTileMap>(Vec2i{ (int)tiles.column_count(), (int)tiles.row_count() });
		for (auto it = tiles.begin(); it != tiles.end(); it++) {
			if (!it->empty()) {
				collider->setTile({ (int)it.column(), (int)it.row() }, TileShape::from_string(*it));
			}
		}
		collider->applyChanges();
	}
};

TEST_F(surfacetracker, stick_slope_down)
{
	// Goal: Surface tracker should stick to slopes when slope_sticking is set.
	// Action: slide down a slope and don't lose contact on any frame

	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"",			"",			"",			"",			""},
		/* y:16_*/ {"",			"",			"",			"",			""},		
		/* y:32_*/ {"solid",	"solid",	"slope-h",	"",			""},
		/* y:48_*/ {"solid",	"solid",	"solid",	"slope-h",	""},		
		/* y:64_*/ {"solid",	"solid",	"solid",	"solid",	"slope-h"}, 
	});

	box->teleport({ 8, 32 });
	box->set_vel(Vec2f{ 100.f, 0.f });
	box->set_gravity({ 0, 400 });

	TestPhysRenderer render({ 0, 0, 80, 80 });
	render.draw(colMan);

	while (render.curr_frame < 60 && box->getPosition().x < 80)
	{
		update();
		render.draw(colMan);
		EXPECT_TRUE(ground->has_contact());
	}
}

TEST_F(surfacetracker, stick_on_touch)
{
	// Goal:   Surface tracker should stick to slopes on the very frame we collide.
	// Action: fall onto ground just before, keep sliding without losing contact

	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"",			"",			"",			"",			""},
		/* y:16_*/ {"",			"",			"",			"",			""},
		/* y:32_*/ {"solid",	"solid",	"slope-h",	"",			""},
		/* y:48_*/ {"solid",	"solid",	"solid",	"slope-h",	""},
		/* y:64_*/ {"solid",	"solid",	"solid",	"solid",	"slope-h"},
		});

	box->teleport({ 32 - 5, 31 });
	box->set_vel(Vec2f{ 6.f, 6.f } / one_frame);
	box->set_gravity({ 0, 400 });

	TestPhysRenderer render({0, 0, 80, 80});
	render.draw(colMan);

	while (render.curr_frame < 60 && box->getPosition().x < 80)
	{
		update();
		render.draw(colMan);
		EXPECT_TRUE(ground->has_contact());
	}

}

TEST_F(surfacetracker, friction_slide_to_stop)
{
	// Goal:   Surface tracker should stick to slopes on the very frame we collide.
	// Action: fall onto ground just before, keep sliding without losing contact

	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"",			"",			"",			"",			"",			""},
		/* y:16_*/ {"",			"",			"",			"",			"",			""},
		/* y:32_*/ {"slope-h",	"",			"",			"",			"",			""},
		/* y:48_*/ {"solid",	"slope-h",	"",			"",			"",			""},
		/* y:64_*/ {"solid",	"solid",	"solid",	"solid",	"solid",	"solid"},
		});

	box->teleport({ 8, 32 });
	box->set_vel({0, 200});
	box->set_gravity({ 0, 400 });

	ground->settings.has_friction = true;
	ground->settings.surface_friction.kinetic = 0.6f;
	ground->settings.surface_friction.stationary = 0.9f;

	TestPhysRenderer render(collider->getBoundingBox());
	render.draw(colMan);

	bool contact = false;

	while (render.curr_frame < 120 && (!contact || box->get_vel().x != 0.f))
	{
		update();
		render.draw(colMan);

		if (!contact)
		{
			contact = ground->has_contact();
		}
		else {
			EXPECT_TRUE(ground->has_contact());
		}
	}
	EXPECT_TRUE(contact);
}


TEST_F(surfacetracker, move_platform_lateral)
{

	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"",			"",			"",			"",			"",			""},
		/* y:16_*/ {"",			"",			"",			"",			"",			""},
		/* y:32_*/ {"",			"",			"",			"",			"",			""},
		/* y:48_*/ {"solid",	"solid",	"",			"",			"",			""},
		/* y:64_*/ {"",			"",			"",			"",			"",			""},
	});

	box->teleport({ 32, 32 });
	box->set_vel({ 0, 0 });
	box->set_gravity({ 0, 400 });

	ground->settings.move_with_platforms = true;
	ground->settings.has_friction = true;
	ground->settings.surface_friction.kinetic = 0.6f;
	ground->settings.surface_friction.stationary = 0.9f;

	TestPhysRenderer render(collider->getBoundingBox());
	render.draw(colMan);

	Vec2f dir{ 1.f, 0.f };

	bool contact = false;
	std::optional<Vec2f> rest_offset;

	while (render.curr_frame < 128)
	{
		if (render.curr_frame == 64) {
			dir = Vec2f{ -1.f, 0.f };
		}

		collider->setPosition(collider->getPosition() + dir);
		ff::Vec2f nVel = (collider->getPosition() - collider->getPrevPosition()) / one_frame;

		collider->delta_velocity = nVel - collider->velocity;
		collider->velocity = nVel;

		update();
		render.draw(colMan);

		if (!contact)
		{
			contact = ground->has_contact();
		}
		else {
			EXPECT_TRUE(ground->has_contact());
			if (!rest_offset && box->get_vel().x == 0.f)
			{
				rest_offset = box->getPosition() - collider->getPosition();
			}
			else if (rest_offset) {
				Vec2f off = box->getPosition() - collider->getPosition();
				EXPECT_FLOAT_EQ(rest_offset->x, off.x);
				EXPECT_FLOAT_EQ(rest_offset->y, off.y);
			}
		}

	}
	EXPECT_TRUE(contact);
	EXPECT_TRUE(rest_offset.has_value());
}

TEST_F(surfacetracker, move_platform_vertical)
{
	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"",			"",			""},
		/* y:16_*/ {"",			"",			""},
		/* y:32_*/ {"",			"",			""},
		/* y:48_*/ {"",			"solid",	""},
		/* y:64_*/ {"",			"",			""},
				   {"",			"",			""},
				   {"",			"",			""},
				   {"",			"",			""},
				   {"",			"",			""},
				   {"",			"",			""},
				   {"",			"",			""},
		});

	box->teleport({ 24, 32 });
	box->set_vel({ 0, 0 });
	box->set_gravity({ 0, 400 });

	ground->settings.move_with_platforms = true;
	ground->settings.has_friction = true;
	ground->settings.surface_friction.kinetic = 0.6f;
	ground->settings.surface_friction.stationary = 0.9f;

	TestPhysRenderer render(collider->getBoundingBox());
	render.draw(colMan);

	Vec2f dir{ 0.f, 2.f };

	bool contact = false;
	std::optional<Vec2f> rest_offset;

	while (render.curr_frame < 150)
	{
		if (render.curr_frame == 50) {
			dir = Vec2f{ 0.f, -2.f };
		}
		if (render.curr_frame == 100) {
			dir = Vec2f{ 0.f, 2.f };
		}

		collider->setPosition({ sinf((float)render.curr_frame / 5.f) * 8.f, collider->getPosition().y + dir.y });
		ff::Vec2f nVel = (collider->getPosition() - collider->getPrevPosition()) / one_frame;

		collider->delta_velocity = nVel - collider->velocity;
		collider->velocity = nVel;

		update();
		render.draw(colMan);

		if (!contact)
		{
			contact = ground->has_contact();
		}
		else {
			EXPECT_TRUE(ground->has_contact());
			if (!rest_offset && box->get_vel().x == 0.f)
			{
				rest_offset = box->getPosition() - collider->getPosition();
			}
			else if (rest_offset) {
				Vec2f off = box->getPosition() - collider->getPosition();
				EXPECT_FLOAT_EQ(rest_offset->x, off.x);
				EXPECT_FLOAT_EQ(rest_offset->y, off.y);
			}
		}
	}
	EXPECT_TRUE(contact);
	EXPECT_TRUE(rest_offset.has_value());

}
