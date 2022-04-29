
#include "fastfall/game/Instance.hpp"
#include "fastfall/game/object/ObjectComponents.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderTileMap.hpp"
#include "fastfall/game/phys/Collidable.hpp"

#include "TestPhysRenderer.hpp"
#include "gtest/gtest.h"

using namespace ff;


class collision : public ::testing::Test {

protected:
	CollisionManager colMan;
	Collidable* box;
	ColliderTileMap* collider = nullptr;

	static constexpr secs one_frame = (1.0 / 60.0);

	collision()
		: colMan{ 0u }
	{
	}

	virtual ~collision() {
	}

	void SetUp() override {
		Vec2f pos = { 0, 0 };
		Vec2f size = { 16, 32 };
		Vec2f grav = { 0, 0 };
		box = colMan.create_collidable(pos, size, grav);
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

	void initTileMap(ColliderTileMap* map, grid_vector<std::string_view> tiles)
	{
		for (auto it = tiles.begin(); it != tiles.end(); it++) {
			if (!it->empty()) {
				map->setTile({ (int)it.column(), (int)it.row() }, TileShape::from_string(*it));
			}
		}
		map->applyChanges();
	}
};

TEST_F(collision, wall_to_ceil_clip)
{
	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"solid",	"solid",	"",			"",			""},
		/* y:16_*/ {"solid",	"solid",	"",			"",			""},
		/* y:32_*/ {"solid",	"solid",	"solid",	"solid",	"solid"},
		/* y:48_*/ {"",			"",			"",			"",			""},
		/* y:64_*/ {"",			"",			"",			"",			""},
		});

	box->teleport(Vec2f{ 72, 32 });
	box->set_vel(Vec2f{ -800.f, 0.f });
	box->set_gravity(Vec2f{ 0.f, 500.f });

	TestPhysRenderer render({ 0, 0, 80, 80 });
	render.frame_delay = 2;
	render.draw(colMan);

	bool hitwall = false;

	while (render.curr_frame < 20) {

		box->set_vel(Vec2f{ -800.f, 0.f });
		update();
		render.draw(colMan);

		if (!hitwall) {
			hitwall = box->get_contacts().size() == 2;
		}

		if (hitwall) {
			EXPECT_EQ(box->get_contacts().size(), 2);
			EXPECT_LE(box->getPosition().y, 32.f);
		}
	}
	EXPECT_TRUE(hitwall);
}

TEST_F(collision, ghostcheck_slopedceil_to_wall)
{
	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"solid",	"",			"",			"",			""},
		/* y:16_*/ {"slope-hv",	"",			"",			"",			""},
		/* y:32_*/ {"",			"",			"",			"",			""},
		/* y:48_*/ {"solid",	"solid",	"",			"",			""},
		/* y:64_*/ {"",			"",			"",			"",			""},
		});

	box->teleport(Vec2f{ 20, 48 });
	box->set_vel(Vec2f{ 0.f, -8.f  } / one_frame);

	TestPhysRenderer render({ 0, 0, 80, 80 });
	render.frame_delay = 20;
	render.draw(colMan);

	update();
	render.draw(colMan);

	EXPECT_EQ(box->get_contacts().size(), 1);

	const auto& contact = box->get_contacts().at(0);
	EXPECT_TRUE(contact.hasContact);
	EXPECT_TRUE(contact.ortho_normal == Vec2f(0.f, 1.f) );
	EXPECT_TRUE(contact.collider_normal == Vec2f(1.f, 1.f).unit() );
	EXPECT_EQ(contact.impactTime, 0.5f);


	while (render.curr_frame < 8) {
		update();
		render.draw(colMan);
	}

}

TEST_F(collision, half_pipe)
{
	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"", 		"",			"",			"",			"",			""},
		/* y:16_*/ {"", 		"",			"",			"",			"",			"solid"},
		/* y:32_*/ {"", 		"",			"",			"",			"steep2",	"solid"},
		/* y:48_*/ {"", 		"",			"",			"",			"steep1",   "solid"},
		/* y:64_*/ {"", 		"",			"",			"slope",	"solid",	"solid"},
		/* y:80_*/ {"half", 	"half",		"shallow2",	"solid",	"solid",	"solid"},
		/* y:96_*/ {"solid", 	"solid",	"solid",	"solid",	"solid",	"solid"},
		});

	box->teleport(Vec2f{ 8, 88 });
	box->set_vel(Vec2f{ 400.f, 0.f  });
	box->set_gravity(Vec2f{ 0.f, 500.f  });

	TestPhysRenderer render(collider->getBoundingBox());
	render.frame_delay = 2;
	render.draw(colMan);

	while (render.curr_frame < 120) {
		update();
		render.draw(colMan);
	}
	ASSERT_LT(box->getPosition().x, 0.f);

}

TEST_F(collision, slip_v)
{
	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"", 		"",			""},
		/* y:16_*/ {"", 		"",			""},
		/* y:32_*/ {"solid", 	"",			""},
		});

	box->teleport(Vec2f{ 40, 32 + 5 });
	box->set_vel(Vec2f{ -400.f, 0.f  });
	box->set_gravity(Vec2f{ 0.f, 0.f  });
	box->setSlip({Collidable::SlipState::SlipVertical, 5.f});

	TestPhysRenderer render(collider->getBoundingBox());
	render.frame_delay = 20;
	render.draw(colMan);

	while (render.curr_frame < 8) {
		update();
		render.draw(colMan);
	}
	ASSERT_LT(box->getPosition().x, 0.f);

	box->teleport(Vec2f{ 40, 32 + 5.01f });

	while (render.curr_frame < 16) {
		update();
		render.draw(colMan);
	}
	ASSERT_EQ(box->getPosition().x, 24.f);

}

TEST_F(collision, static_tunneling)
{
	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"",			"",			"halfvert-h", "",			""},
		/* y:16_*/ {"",			"",			"halfvert-h", "",			""},
		/* y:32_*/ {"",			"",			"halfvert-h", "",			""},
		});

	box->teleport(Vec2f{ 8, 40 });
	box->set_vel(Vec2f{ 5000.f, 0.f }); // very fast


	TestPhysRenderer render(collider->getBoundingBox());
	render.frame_delay = 20;
	render.draw(colMan);

	while (render.curr_frame < 8) {

		box->set_vel(Vec2f{ 5000.f, 0.f });
		update();
		render.draw(colMan);

		EXPECT_EQ(box->get_contacts().size(), 1);

		if (!box->get_contacts().empty()) {
			const auto& contact = box->get_contacts().at(0);
			EXPECT_TRUE(contact.hasContact);
			EXPECT_TRUE(contact.ortho_normal == Vec2f(-1.f, 0.f));
			EXPECT_TRUE(contact.collider_normal == Vec2f(-1.f, 0.f));
		}
	}
}

TEST_F(collision, moving_tunneling)
{
	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"",			"",			"",			"",			"halfvert-h"},
		/* y:16_*/ {"",			"",			"",			"",			"halfvert-h"},
		/* y:32_*/ {"",			"",			"",			"",			"halfvert-h"},
		});

	box->teleport(Vec2f{ 8, 40 });
	box->set_vel(Vec2f{ 5000.f, 0.f });

	TestPhysRenderer render(math::rect_extend(collider->getBoundingBox(), Cardinal::W, 64.f));
	render.frame_delay = 20;
	render.draw(colMan);

	while (render.curr_frame < 8) {

		collider->setPosition(collider->getPosition() + Vec2f{ -32.f, 0.f });
		ff::Vec2f nVel = (collider->getPosition() - collider->getPrevPosition()) / one_frame;
		collider->delta_velocity = nVel - collider->velocity;
		collider->velocity = nVel;

		box->set_vel(Vec2f{ 5000.f, 0.f });
		update();
		render.draw(colMan);

		EXPECT_EQ(box->get_contacts().size(), 1);

		if (!box->get_contacts().empty()) {
			const auto& contact = box->get_contacts().at(0);
			EXPECT_TRUE(contact.hasContact);
			EXPECT_TRUE(contact.ortho_normal == Vec2f(-1.f, 0.f));
			EXPECT_TRUE(contact.collider_normal == Vec2f(-1.f, 0.f));
		}
	}
}

TEST_F(collision, wedge_against_floor_right)
{
	initTileMap({
		{"",			"",			"",			""},
		{"",			"",			"",			""},
		{"",			"",			"",			""},
		{"",			"",			"",			""},
		{"solid",	"solid",	"solid",	"solid"},
	});

	grid_vector<std::string_view> wedge_tiles{
		{ "solid", 		"solid", 	""},
		{ "solid", 		"slope-hv", ""},
		{ "slope-hv", 	"", 		""},
	};

	auto wedge = colMan.create_collider<ColliderTileMap>(
			Vec2i{ (int)wedge_tiles.column_count(), (int)wedge_tiles.row_count() }
		);
	initTileMap(wedge, wedge_tiles);
	wedge->teleport(Vec2f{0.f, -16.f});

	box->teleport(Vec2f{ 8, 64 });
	box->set_gravity(Vec2f{0.f, 500.f});

	TestPhysRenderer render(collider->getBoundingBox());
	render.frame_delay = 2;
	render.draw(colMan);

	while (render.curr_frame < 60) {

		Vec2f vel{ 0.f, 50.f };
		wedge->setPosition(wedge->getPosition() + (vel * one_frame));
		wedge->delta_velocity = vel - wedge->velocity;
		wedge->velocity = vel;
		wedge->update(one_frame);

		Vec2f vel2{ 0.f, 0.f };
		collider->setPosition(collider->getPosition() + (vel2 * one_frame));
		collider->delta_velocity = vel2 - collider->velocity;
		collider->velocity = vel2;

		update();
		render.draw(colMan);
		fmt::print(stderr, "{}\n", box->get_vel());
		
		if (render.curr_frame > 24 && box->getPosition().x < 64) {
			fmt::print(stderr, "{:08b}\n", box->get_state_flags().value());
			EXPECT_GT(box->get_vel().x, 0.f);
			//EXPECT_TRUE(box->get_state_flags().has_set(collision_state_t::flags::Floor));
		}
	}
}


TEST_F(collision, floor_into_wedge_left)
{
	initTileMap({
		{"",			"",			"slope-v",	"solid"},
		{"",			"",			"",			"slope-v"},
		{"half",		"half",		"",			""},
		{"half-v",		"half-v",	"",			""},
		{"",			"",			"",			""},
		});

	grid_vector<std::string_view> floor_tiles{
		{ "", 		""},
		{ "solid", 		"solid"},
	};

	auto floor = colMan.create_collider<ColliderTileMap>(
		Vec2i{ (int)floor_tiles.column_count(), (int)floor_tiles.row_count() }
	);
	initTileMap(floor, floor_tiles);
	floor->teleport(Vec2f{ 32.f, 64.f });

	box->teleport(Vec2f{ 56, 64 });
	box->set_gravity(Vec2f{ 0.f, 500.f });

	TestPhysRenderer render(collider->getBoundingBox());
	render.frame_delay = 2;
	render.draw(colMan);

	while (render.curr_frame < 60) {

		//Vec2f vel{ 0.f, -50.f };
		Vec2f vel{ 0.f, (float)std::max(-100.0, (0.f - floor->getPosition().y) / one_frame) };

		floor->setPosition(floor->getPosition() + (vel * one_frame));
		floor->delta_velocity = vel - floor->velocity;
		floor->velocity = vel;
		floor->update(one_frame);

		update();
		render.draw(colMan);
		fmt::print(stderr, "{}\n", box->get_vel());


		if (render.curr_frame > 24 && box->getPosition().x > 0) {
			fmt::print(stderr, "{:08b}\n", box->get_state_flags().value());
			EXPECT_LT(box->get_vel().x, 0.f);
		}
	}
}
