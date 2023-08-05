
#include "fastfall/game/phys/collider_regiontypes/ColliderTileMap.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/World.hpp"

#include "TestPhysRenderer.hpp"
#include "gtest/gtest.h"

#include "nlohmann/json.hpp"
#include <fstream>

using namespace ff;


class collision : public ::testing::Test {

protected:
	CollisionSystem* colMan;
    World world;
    ID<Entity> collider_obj_id;
    ID<Entity> collidable_obj_id;
	Collidable* box = nullptr;
	ColliderTileMap* collider = nullptr;
	std::fstream log;

	static constexpr secs one_frame = (1.0 / 60.0);

	nlohmann::ordered_json data;

	collision() {
    };

	virtual ~collision() 
	{
		std::string test_log_name = fmt::format("phys_render_out/{}__{}.log",
			::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name(),
			::testing::UnitTest::GetInstance()->current_test_info()->name());

		log.open(test_log_name,
			std::ios_base::out
			| std::ios_base::trunc
		);

		log << data.dump(4) << std::endl;

		log.close();
	}

	void SetUp() override {
		Vec2f pos = { 0, 0 };
		Vec2f size = { 16, 32 };
		Vec2f grav = { 0, 0 };
        //auto id = world.create_entity();
        collidable_obj_id = world.create_entity();
        collider_obj_id = world.create_entity();
		box = world.create<Collidable>(collidable_obj_id, pos, size, grav).ptr;
        colMan = &world.system<CollisionSystem>();
	}

	void TearDown() override
	{
	}

	void update()
	{
		colMan->dumpCollisionDataThisFrame(&data[colMan->getFrameCount()]);
		colMan->update(world, one_frame);
	}

	void initTileMap(grid_vector<std::string_view> tiles)
	{
		collider =
                world.create<ColliderTileMap>(
                        collider_obj_id,
                        Vec2i{ (int)tiles.column_count(), (int)tiles.row_count() }
                    ).ptr;
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

TEST_F(collision, minimal)
{
    initTileMap({
            /*          x:0
            /* y:0 _*/ {""},
            /* y:16_*/ {""},
            /* y:32_*/ {"solid"},
    });

    box->teleport(Vec2f{ 8, 32 });
    box->set_local_vel(Vec2f{ 0.f, 0.f });
    box->set_gravity(Vec2f{ 0.f, 500.f });

    TestPhysRenderer render(world, { 0, 0, 16, 48 });
    render.frame_delay = 2;
    render.draw();

    while (render.curr_frame < 10) {
        update();
        render.draw();

        EXPECT_EQ(box->get_contacts().size(), 1);
    }
}

TEST_F(collision, wall_to_ceil_clip)
{
	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"",			"",			"",			"",			""},
		/* y:16_*/ {"solid",	"solid",	"",			"",			""},
		/* y:32_*/ {"solid",	"solid",	"solid",	"solid",	"solid"},
		/* y:48_*/ {"",			"",			"",			"",			""},
		/* y:64_*/ {"",			"",			"",			"",			""},
		});

	box->teleport(Vec2f{ 40, 32 });
	box->set_local_vel(Vec2f{ -800.f, 0.f });
	box->set_gravity(Vec2f{ 0.f, 500.f });

	TestPhysRenderer render(world, { 0, 0, 80, 80 });
	render.frame_delay = 2;
	render.draw();

	bool hitwall = false;

	while (render.curr_frame < 20) {

		box->set_local_vel(Vec2f{ -800.f, 0.f });
		update();
		render.draw();

		hitwall = box->getPosition().x <= 40.f;

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
	box->set_local_vel(Vec2f{ 0.f, -8.f  } / one_frame);

	TestPhysRenderer render(world, { 0, 0, 80, 80 });
	render.frame_delay = 20;
	render.draw();

	update();
	render.draw();

	EXPECT_EQ(box->get_contacts().size(), 1);

	const auto& contact = box->get_contacts().at(0);
	EXPECT_TRUE(contact.hasContact);
	EXPECT_TRUE(contact.ortho_n == Vec2f(0.f, 1.f) );
	EXPECT_TRUE(contact.collider_n == Vec2f(1.f, 1.f).unit() );
	EXPECT_EQ(contact.impactTime, 0.5f);


	while (render.curr_frame < 2) {
		update();
		render.draw();
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
	box->set_local_vel(Vec2f{ 400.f, 0.f  });
	box->set_gravity(Vec2f{ 0.f, 500.f  });

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.frame_delay = 2;
	render.draw();

	while (render.curr_frame < 120) {
		update();
		render.draw();
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
	box->set_local_vel(Vec2f{ -400.f, 0.f  });
	box->set_gravity(Vec2f{ 0.f, 0.f  });
	box->setSlip({Collidable::SlipState::SlipVertical, 5.f});

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.frame_delay = 20;
	render.draw();

	while (render.curr_frame < 4) {
		update();
		render.draw();
	}
	ASSERT_EQ(box->getPosition().y, 32.f);

	box->teleport(Vec2f{ 40, 32 + 5.01f });

	while (render.curr_frame < 8) {
		update();
		render.draw();
	}
	ASSERT_GT(box->getPosition().y, 32.f);

}

TEST_F(collision, slip_v_oneway)
{
    initTileMap({
    /*          x:0         x:16		x:32		x:48		x:64 */
    /* y:0  */ {"", 		"",			""},
    /* y:16 */ {"", 		"",			""},
    /* y:32 */ {"oneway", 	"",			""},
    });

    box->teleport(Vec2f{ 40, 32 + 5 });
    box->set_local_vel(Vec2f{ -400.f, 0.f  });
    box->set_gravity(Vec2f{ 0.f, 0.f  });
    box->setSlip({Collidable::SlipState::SlipVertical, 5.f});

    TestPhysRenderer render(world, collider->getBoundingBox());
    render.frame_delay = 20;
    render.draw();

    while (render.curr_frame < 4) {
        update();
        render.draw();
    }
    ASSERT_EQ(box->getPosition().y, 32.f);

    box->teleport(Vec2f{ 40, 32 + 5.01f });

    while (render.curr_frame < 8) {
        update();
        render.draw();
    }
    ASSERT_GT(box->getPosition().y, 32.f);

}


TEST_F(collision, floor_to_steep)
{
	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"", 		"",			""},
		/* y:16_*/ {"", 		"",			""},
		/* y:32_*/ {"solid", 	"steep1-h",	""},
		});

	box->teleport(Vec2f{ 15.9f, 32 });
	box->set_local_vel(Vec2f{ 50.f, 0.f });
	box->set_gravity(Vec2f{ 0.f, 500.f });

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.frame_delay = 20;
	render.draw();

	update();
	render.draw();

	ASSERT_EQ(box->getPosition().y, 32.f);
}

// TUNNELING

TEST_F(collision, tunneling_static)
{
	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"",			"",			"halfvert-h", "",			""},
		/* y:16_*/ {"",			"",			"halfvert-h", "",			""},
		/* y:32_*/ {"",			"",			"halfvert-h", "",			""},
		});

	box->teleport(Vec2f{ 8, 40 });
	box->set_local_vel(Vec2f{ 5000.f, 0.f }); // very fast


	TestPhysRenderer render(world, collider->getBoundingBox());
	render.frame_delay = 20;
	render.draw();

	while (render.curr_frame < 8) {

		box->set_local_vel(Vec2f{ 5000.f, 0.f });
		update();
		render.draw();

		EXPECT_EQ(box->get_contacts().size(), 1);

		if (!box->get_contacts().empty()) {
			const auto& contact = box->get_contacts().at(0);
			EXPECT_TRUE(contact.hasContact);
			EXPECT_TRUE(contact.ortho_n == Vec2f(-1.f, 0.f));
			EXPECT_TRUE(contact.collider_n == Vec2f(-1.f, 0.f));
		}
	}
}

TEST_F(collision, tunneling_moving)
{
	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"",			"",			"",			"",			"halfvert-h"},
		/* y:16_*/ {"",			"",			"",			"",			"halfvert-h"},
		/* y:32_*/ {"",			"",			"",			"",			"halfvert-h"},
		});

	box->teleport(Vec2f{ 8, 40 });
	box->set_local_vel(Vec2f{ 5000.f, 0.f });

	TestPhysRenderer render(world, math::rect_extend(collider->getBoundingBox(), Cardinal::W, 64.f));
	render.frame_delay = 20;
	render.draw();

	while (render.curr_frame < 8) {

		collider->setPosition(collider->getPosition() + Vec2f{ -32.f, 0.f });
		ff::Vec2f nVel = (collider->getPosition() - collider->getPrevPosition()) / one_frame;
		collider->delta_velocity = nVel - collider->velocity;
		collider->velocity = nVel;

		box->set_local_vel(Vec2f{ 5000.f, 0.f });
		update();
		render.draw();

		EXPECT_EQ(box->get_contacts().size(), 1);

		if (!box->get_contacts().empty()) {
			const auto& contact = box->get_contacts().at(0);
			EXPECT_TRUE(contact.hasContact);
			EXPECT_TRUE(contact.ortho_n == Vec2f(-1.f, 0.f));
			EXPECT_TRUE(contact.collider_n == Vec2f(-1.f, 0.f));
		}
	}
}

// WEDGES

TEST_F(collision, wedge_against_floor_right)
{
	initTileMap({
		{"",			"",			"",			""},
		{"",			"",			"",			""},
		{"",			"",			"",			""},
		{"",			"",			"",			""},
		{"shallow1",	"shallow2",	"solid",	"solid"},
	});
    assert(collider != nullptr);

	grid_vector<std::string_view> wedge_tiles{
		{ "solid", 		"solid", 	""},
		{ "solid", 		"slope-hv", ""},
		{ "slope-hv", 	"", 		""},
	};

	auto wedge = world.create<ColliderTileMap>(
                world.create_entity(),
			    Vec2i{ (int)wedge_tiles.column_count(), (int)wedge_tiles.row_count() }
		    ).ptr;
	initTileMap(wedge, wedge_tiles);
	wedge->teleport(Vec2f{0.f, -16.f});

	box->teleport(Vec2f{ 8, 64 });
	box->set_gravity(Vec2f{0.f, 500.f});

	TestPhysRenderer render(world, math::rect_extend(collider->getBoundingBox(), Cardinal::E, 64.f));
	render.frame_delay = 2;
	render.draw();

	while (render.curr_frame < 120) {

		Vec2f vel{ 0.f, 50.f };
		wedge->setPosition(wedge->getPosition() + (vel * one_frame));
		wedge->delta_velocity = vel - wedge->velocity;
		wedge->velocity = vel;
		//wedge->update(one_frame);

		Vec2f vel2{ 0.f, -10.f };
		collider->setPosition(collider->getPosition() + (vel2 * one_frame));
		collider->delta_velocity = vel2 - collider->velocity;
		collider->velocity = vel2;

		update();
		render.draw();

		if (render.curr_frame > 24 && box->getPosition().x < 32) {
			EXPECT_GT(box->get_local_vel().x, 0.f);
			EXPECT_TRUE(box->get_state_flags().has_set(collision_state_t::flags::Floor));
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

	auto floor = world.create<ColliderTileMap>(
            world.create_entity(),
            Vec2i{ (int)floor_tiles.column_count(), (int)floor_tiles.row_count() }
        ).ptr;
	initTileMap(floor, floor_tiles);
	floor->teleport(Vec2f{ 32.f, 64.f });

	box->teleport(Vec2f{ 56, 64 });
	box->set_gravity(Vec2f{ 0.f, 500.f });

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.frame_delay = 2;
	render.draw();

	while (render.curr_frame < 60) {

		//Vec2f vel{ 0.f, -50.f };
		Vec2f vel{ 0.f, (float)std::max(-100.0, (0.f - floor->getPosition().y) / one_frame) };

		floor->setPosition(floor->getPosition() + (vel * one_frame));
		floor->delta_velocity = vel - floor->velocity;
		floor->velocity = vel;
		//floor->update(one_frame);

		update();
		render.draw();

		if (render.curr_frame > 24 && box->getPosition().x > 0) {
			EXPECT_LT(box->get_local_vel().x, 0.f);
		}
	}
}

TEST_F(collision, wedge_with_oneway_floor) {

	initTileMap({
		{"solid", 		"shallow2-hv",	"shallow1-hv"},
		{"shallow1-hv", "",				""},
		{"oneway", 		"oneway",		"oneway"},
	});

	box->setSize({ 16, 24 });
	box->teleport({ 32, 32 });
	box->set_local_vel({ -100, -100 });
	box->set_gravity({ 0, 500 });

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.frame_delay = 20;
	render.draw();

	while (render.curr_frame < 2) {
		update();
		render.draw();
		EXPECT_EQ(box->getPosition().y, 32.f);
	}
}

TEST_F(collision, wedge_with_oneway_ceil) {
	initTileMap({
		{"oneway-v", 	"oneway-v", 	"oneway-v",		"oneway-v"},
		{"", 			"", 			"",				"shallow1"},
		{"", 			"shallow1", 	"shallow2",		"solid"},
		{"shallow2",	"solid", 		"solid",		"solid"},
	});

	box->setSize({ 16, 24 });
	box->teleport({ 0, 48 });
	box->set_local_vel({ 500, 0 });
	box->set_gravity({ 0, 500 });

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.frame_delay = 8;
	render.draw();

	while (render.curr_frame < 20) {
		box->set_local_vel({ 500, 0 });
		update();
		render.draw();

		if (render.curr_frame >= 4) {
			EXPECT_GE(box->getPosition().y, 40.f);
		}
	}
}

TEST_F(collision, into_steep_slope) 
{
	initTileMap({
		{"", 			"", 			""},
		{"", 			"", 			""},
		{"", 			"", 			"steep1"},
		{"", 			"steep2", 		"solid"},
		{"", 			"steep1", 		"solid"},
		{"solid",		"solid", 		"solid"},
		});


	box->setSize({ 16, 32 });
	box->teleport({ 8, 80 });
	box->set_local_vel({ 200, 0 });
	box->set_gravity({ 0, 50 });

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.frame_delay = 2;
	render.draw();

	while (render.curr_frame < 30) {
		box->add_accel({ 500, 0 });
		int fcount = colMan->getFrameCount();
		update();
		render.draw();

		if (box->getPosition().x < 40) {

			bool has_collision = box->get_state_flags().has_any(
				collision_state_t::flags::Floor,
				collision_state_t::flags::Wall_L,
				collision_state_t::flags::Wall_R);

			if (!has_collision)
			{
				fmt::print(stderr, "frame: {}\n", fcount);
			}

			EXPECT_TRUE(has_collision);
		}
	}
}

TEST_F(collision, into_shallow_corner_w_oneway)
{
	initTileMap({
		{"",	"",		"", 			"shallow1",		"shallow2"},
		{"",	"",		"oneway", 		"shallow1-v", 	"shallow2-v"},
		{"",	"",		"", 			"", 			""},
		{"",	"",		"", 			"", 			""},
	});

	box->setSize({ 16, 32 });
	box->teleport({ 52, 64 });
	box->set_local_vel({ -1, -200 });
	box->set_gravity({ 0, 0 });

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.frame_delay = 2;
	render.draw();

	while (render.curr_frame < 30) {

		int fcount = colMan->getFrameCount();
		update();
		render.draw();

		bool has_wall = box->get_state_flags().has_any(
			collision_state_t::flags::Wall_L,
			collision_state_t::flags::Wall_R);

		if (has_wall)
		{
			fmt::print(stderr, "frame: {}\n", fcount);
		}

		EXPECT_FALSE(has_wall);
	}
}

TEST_F(collision, oneway_into_wall)
{
    initTileMap({
        {"",	"",		"", 	"",	    ""},
        {"",	"",		"", 	"solid", ""},
        {"",	"",		"", 	"", 	""},
        {"",	"",		"", 	"", 	""},
    });

    grid_vector<std::string_view> floor_tiles{
        { "oneway", "oneway"},
    };
    auto floor = world.create<ColliderTileMap>(
                world.create_entity(),
                Vec2i{ (int)floor_tiles.column_count(), (int)floor_tiles.row_count() }
            ).ptr;
    initTileMap(floor, floor_tiles);
    floor->teleport(Vec2f{ 0.f, 64.f });

    box->setSize({ 16, 32 });
    box->teleport({ 24, 64 });
    box->set_local_vel(Vec2f{});
    box->set_gravity({ 200, 400 });

    TestPhysRenderer render(world, collider->getBoundingBox());
    render.frame_delay = 2;
    render.draw();

    Vec2f vel{ 20.f, -20.f };

    while (render.curr_frame < 120) {
        floor->setPosition(floor->getPosition() + (vel * one_frame));
        floor->delta_velocity = vel - floor->velocity;
        floor->velocity = vel;
        //floor->update(one_frame);

        bool badframe = colMan->getFrameCount() == 71;

        update();
        render.draw();

        EXPECT_EQ(box->getPosition().y, floor->getPosition().y);
    }
}

TEST_F(collision, up_thru_oneway)
{
    initTileMap({
        {"",	"",		""},
        {"",	"",		""},
        {"",	"oneway",		""},
        {"",	"",		""},
    });

    box->setSize({ 16, 32 });
    box->teleport({ 24, 64 });
    box->set_local_vel({0.f, -170 });
    box->set_gravity({ 0, 400 });

    TestPhysRenderer render(world, collider->getBoundingBox());
    render.frame_delay = 2;
    render.draw();

    while (render.curr_frame < 60) {
        bool can_collide = box->get_local_vel().y >= 0;

        update();
        render.draw();

        if (!can_collide) {
            EXPECT_EQ(box->get_contacts().size(), 0);
        }
    }
}

TEST_F(collision, across_oneway) {

    initTileMap({
        {"",	""},
        {"",	""},
        {"oneway",	"oneway"},
    });

    box->setSize({ 16, 32 });
    box->teleport({ 8, 32 });
    box->set_local_vel({ 1.f, 0.f });
    box->set_gravity({ 0, 400 });

    TestPhysRenderer render(world, collider->getBoundingBox());
    render.frame_delay = 2;
    render.draw();

    while (render.curr_frame < 600) {
        update();
        render.draw();
        EXPECT_EQ(box->get_contacts().size(), 1);
    }
}


