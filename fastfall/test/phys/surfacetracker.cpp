#include "gtest/gtest.h"

#include "fastfall/game/World.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderTileMap.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/collidable/SurfaceTracker.hpp"

#include "TestPhysRenderer.hpp"


#include "nlohmann/json.hpp"
#include <fstream>

using namespace ff;

class surfacetracker : public ::testing::Test {

protected:
    World world;
	CollisionSystem* colMan;

    ID<Entity> collider_obj_id;
    ID<Entity> collidable_obj_id;
	Collidable* box = nullptr;
	SurfaceTracker* ground = nullptr;

	ColliderTileMap* collider = nullptr;
	std::fstream log;

	static constexpr secs one_frame = (1.0 / 60.0);

	nlohmann::ordered_json data;

	surfacetracker() {
    }

	virtual ~surfacetracker() {
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
		
		Vec2f pos  = { 0, 0 };
		Vec2f size = { 16, 32 };
		Vec2f grav = { 0, 0 };

        collidable_obj_id = world.create_entity();
        collider_obj_id = world.create_entity();
        auto box_id = world.create<Collidable>(collidable_obj_id, pos, size, grav);
		box = world.get(box_id);

        box->set_tracker(
            Angle::Degree(-135),
            Angle::Degree(-45)
        );
        ground = box->get_tracker();

        ground->settings = {
            .slope_sticking = true,
            .stick_angle_max = Angle::Degree(90)
        };

        colMan = &world.system<CollisionSystem>();
	}

	void TearDown() override 
	{
        box = nullptr;
        ground = nullptr;
        collider = nullptr;
	}

	void update() 
	{
		colMan->dumpCollisionDataThisFrame(&data[colMan->getFrameCount()]);
		colMan->update(world, one_frame);
	}

	void initTileMap(grid_vector<std::string_view> tiles) 
	{
		collider = world.get(
                world.create<ColliderTileMap>(collider_obj_id, Vec2i{ (int)tiles.column_count(), (int)tiles.row_count() })
                );
		for (auto it = tiles.begin(); it != tiles.end(); it++) {
			if (!it->empty()) {
				collider->setTile({ (int)it.column(), (int)it.row() }, TileShape::from_string(*it));
			}
		}
		collider->applyChanges();
	}
};

TEST_F(surfacetracker, moving_first_contact_horizontal)
{
    initTileMap({
    /*          x:0
    /* y:0 _*/ {""},
    /* y:16_*/ {""},
    /* y:32_*/ {"solid"},
    });

    collider->velocity = Vec2f{50.f, 0.f};

    box->teleport(Vec2f{ 8, 32 });
    box->set_local_vel(Vec2f{ 0.f, 0.f });
    box->set_gravity(Vec2f{ 0.f, 500.f });

    ground->settings.move_with_platforms = true;
    ground->settings.has_friction = true;
    ground->settings.surface_friction.kinetic = 0.6f;
    ground->settings.surface_friction.stationary = 0.9f;

    TestPhysRenderer render(world, { 0, 0, 16, 48 });
    render.frame_delay = 2;
    render.draw();

    update();
    render.draw();

    EXPECT_EQ(box->get_local_vel().x, -50.f);
    EXPECT_EQ(box->get_local_vel().y, 0.f);
    EXPECT_EQ(box->get_parent_vel().x, 50.f);
    EXPECT_EQ(box->get_parent_vel().y, 0.f);
}

TEST_F(surfacetracker, moving_first_contact_vertical)
{
    initTileMap({
    /*          x:0
    /* y:0 _*/ {""},
    /* y:16_*/ {""},
    /* y:32_*/ {"solid"},
    });

    collider->velocity = Vec2f{0.f, 50.f};

    box->teleport(Vec2f{ 8, 32 });
    box->set_local_vel(Vec2f{ 0.f, 0.f });
    box->set_gravity(Vec2f{ 0.f, 500.f });

    ground->settings.move_with_platforms = true;
    ground->settings.has_friction = true;
    ground->settings.surface_friction.kinetic = 0.6f;
    ground->settings.surface_friction.stationary = 0.9f;

    TestPhysRenderer render(world, { 0, 0, 16, 48 });
    render.frame_delay = 2;
    render.draw();

    update();
    render.draw();

    EXPECT_EQ(box->get_local_vel().x, 0.f);
    EXPECT_EQ(box->get_local_vel().y, 0.f);
    EXPECT_EQ(box->get_parent_vel().x, 0.f);
    EXPECT_EQ(box->get_parent_vel().y, 50.f);
}

TEST_F(surfacetracker, stick_slope_down)
{
	// Goal: Surface tracker should stick to slopes when slope_sticking is set.
	// Action: slide down a slope and don't lose contact on any frame

	initTileMap({
		{"",		"",			"",			"",			""},
		{"",		"",			"",			"",			""},		
		{"solid",	"solid",	"slope-h",	"",			""},
		{"solid",	"solid",	"solid",	"slope-h",	""},		
		{"solid",	"solid",	"solid",	"solid",	"slope-h"}, 
	});

	box->teleport({ 8, 32 });
	box->set_local_vel({ 100.f, 0.f });
	box->set_gravity({ 0, 400 });

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.draw();

	while (render.curr_frame < 60 && box->getPosition().x < 80)
	{
		update();
		render.draw();
		EXPECT_TRUE(ground->has_contact());
	}
}

TEST_F(surfacetracker, broken_slope)
{
	initTileMap({
		{"",		"",			"",			"",			"slope"},
		{"",		"",			"",			"slope",	""},
		{"",		"",			"slope",	"",			""},
		{"",		"slope",	"",			"",			""},
		{"slope",	"",			"",			"",			""},
	});

	box->teleport({ 48.1, 32 });
    box->setPosition({ 47.9, 32.3  });
	box->set_local_vel({ -150.f, 150.f });
	box->set_gravity({ 0, 400 });

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.draw();

    while (render.curr_frame < 15) {
        update();
        render.draw();
    }

    EXPECT_TRUE(ground->has_contact());

}

TEST_F(surfacetracker, stick_on_touch)
{
	// Goal:   Surface tracker should stick to slopes on the very frame we collide.
	// Action: fall onto ground just before, keep sliding without losing contact

	initTileMap({
		{"",		"",			"",			"",			""},
		{"",		"",			"",			"",			""},
		{"solid",	"solid",	"slope-h",	"",			""},
		{"solid",	"solid",	"solid",	"slope-h",	""},
		{"solid",	"solid",	"solid",	"solid",	"slope-h"},
	});

	box->teleport({ 32 - 5, 31 });
	box->set_local_vel(Vec2f{ 6.f, 6.f } / one_frame);
	box->set_gravity({ 0, 400 });

	TestPhysRenderer render(world, {0, 0, 80, 80});
	render.draw();

	while (render.curr_frame < 60 && box->getPosition().x < 80)
	{
		update();
		render.draw();
		EXPECT_TRUE(ground->has_contact());
	}
}

TEST_F(surfacetracker, friction_slide_to_stop)
{
	// Goal:   Surface tracker should stick to slopes on the very frame we collide.
	// Action: fall onto ground just before, keep sliding without losing contact

	initTileMap({
		{"",		"",			"",			"",			"",			""},
		{"",		"",			"",			"",			"",			""},
		{"slope-h",	"",			"",			"",			"",			""},
		{"solid",	"slope-h",	"",			"",			"",			""},
		{"solid",	"solid",	"solid",	"solid",	"solid",	"solid"},
	});

	box->teleport({ 8, 32 });
	box->set_local_vel({0, 200});
	box->set_gravity({ 0, 400 });

	ground->settings.has_friction = true;
	ground->settings.surface_friction.kinetic = 0.6f;
	ground->settings.surface_friction.stationary = 0.9f;

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.draw();

	bool contact = false;

	while (render.curr_frame < 120 && (!contact || box->get_local_vel().x != 0.f))
	{
		update();
		render.draw();

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
		{"",		"",			"",			"",			"",			""},
		{"",		"",			"",			"",			"",			""},
		{"",		"",			"",			"",			"",			""},
		{"solid",	"solid",	"",			"",			"",			""},
		{"",		"",			"",			"",			"",			""},
	});

	box->teleport({ 32, 32 });
	box->set_local_vel({ 0, 0 });
	box->set_gravity({ 0, 400 });

	ground->settings.move_with_platforms = true;
	ground->settings.has_friction = true;
	ground->settings.surface_friction.kinetic = 0.6f;
	ground->settings.surface_friction.stationary = 0.9f;

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.draw();

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
		render.draw();

		if (!contact)
		{
			contact = ground->has_contact();
		}
		else {
			EXPECT_TRUE(ground->has_contact());
			if (!rest_offset && box->get_local_vel().x == 0.f)
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
		{"",			"",			""},
		{"",			"",			""},
		{"",			"",			""},
		{"",			"solid",	""},
		{"",			"",			""},
		{"",			"",			""},
		{"",			"",			""},
		{"",			"",			""},
		{"",			"",			""},
		{"",			"",			""},
		{"",			"",			""},
	});

	box->teleport({ 24, 32 });
	box->set_local_vel({ 0, 0 });
	box->set_gravity({ 0, 400 });

	ground->settings.move_with_platforms = true;
	ground->settings.has_friction = true;
	ground->settings.surface_friction.kinetic = 0.6f;
	ground->settings.surface_friction.stationary = 0.9f;

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.draw();

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
		render.draw();

		if (!contact)
		{
			contact = ground->has_contact();
		}
		else {
			EXPECT_TRUE(ground->has_contact());
			if (!rest_offset && box->get_local_vel().x == 0.f)
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

TEST_F(surfacetracker, higrav_launch_off_hill)
{
	initTileMap({
		{"",		"",			"",			"",			""},
		{"",		"",			"",			"",			""},
		{"",		"",			"",			"",			""},
		{"",		"",			"slope",	"slope-h",	""},
		{"solid",	"solid",	"solid",	"solid",	"solid"},
		});

	box->teleport({ 7, 64 });
	box->set_gravity({ 0, 2000 });
	ground->traverse_set_speed(220.f);

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.draw();
		
	while (render.curr_frame < 120 && box->getPosition().x < 80)
	{
		ground->traverse_set_speed(220.f); // this should not stick to the reverse side of the hill
		update();
		render.draw();

		if (box->getPosition().x <= 48)
		{
			EXPECT_TRUE(ground->has_contact());
		}
		else if (box->getPosition().x <= 75) {
			EXPECT_TRUE(!ground->has_contact());
		}
	}
}

TEST_F(surfacetracker, uphill)
{
	initTileMap({
		{"",		"",			""},
		{"",		"",			""},
		{"",		"slope",	"solid"},
		{"solid",	"solid",	"solid"},
		});

	box->teleport({ 8, 48 });
	box->set_gravity({ 0, 500 });
	ground->traverse_set_speed(50.f);

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.draw();

	while (render.curr_frame < 120 && box->getPosition().x < 48)
	{
		ground->traverse_set_speed(20.f); // this should not stick to the reverse side of the hill
		update();

		int contacts = 0;
		for (auto [cid, col] : world.all<Collidable>()) {
            const auto& arbiters = colMan->get_arbiter(cid);
			for (const auto& [rid, rarb] : arbiters.region_arbiters) {
				contacts += rarb.getQuadArbiters().size();
			}
		}
		render.draw();

		EXPECT_TRUE(ground->has_contact());
	}
}

TEST_F(surfacetracker, uphill_oneway)
{
	initTileMap({
		{"",		"",			""},
		{"",		"",			""},
		{"",		"slope",	"oneway"},
		{"solid",	"solid",	"solid"},
		});

	box->teleport({ 8, 48 });
	box->set_gravity({ 0, 500 });
	ground->traverse_set_speed(50.f);

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.draw();

	while (render.curr_frame < 120 && box->getPosition().x < 48)
	{
		ground->traverse_set_speed(50.f); // this should not stick to the reverse side of the hill
		update();

		int contacts = 0;
        for (auto [cid, col] : world.all<Collidable>()) {
            const auto& arbiters = colMan->get_arbiter(cid);
            for (const auto& [rid, rarb] : arbiters.region_arbiters) {
                contacts += rarb.getQuadArbiters().size();
            }
        }
		render.draw();

		EXPECT_TRUE(ground->has_contact());
	}
}

TEST_F(surfacetracker, peak_of_slope_to_slope)
{
	initTileMap({
		{"",		"",			""},
		{"",		"",			""},
		{"",		"slope-h",	""},
		{"solid",	"solid",	"solid"},
		});

	box->teleport({ 9, 32 });
	box->set_gravity({ 0, 200 });
	ground->traverse_add_accel(500.f);

	TestPhysRenderer render(world, collider->getBoundingBox());
	render.draw();

	while (render.curr_frame < 120 && box->getPosition().x < 48)
	{
		//ground->traverse_set_speed(300.f);
		ground->traverse_add_accel(500.f);
		update();
		render.draw();

		EXPECT_TRUE(ground->has_contact());
	}
}

TEST_F(surfacetracker, into_moving_plat_wall)
{
    initTileMap({
        {"",		"",			"", "", "", "", "", ""},
        {"solid",	"",			"", "", "", "", "", ""},
        {"solid",	"",	        "", "", "", "", "", ""},
        {"solid",	"solid",	"", "", "", "", "", ""},
    });

    box->teleport({ 24, 48 });
    box->set_gravity({ 0, 200 });
    ground->traverse_add_accel(-500.f);
    ground->traverse_set_speed(-100);

    TestPhysRenderer render(world, collider->getBoundingBox());
    render.draw();

    Vec2f dir = Vec2f{ 1.f, 0.f };
    while (render.curr_frame < 60)
    {
        collider->setPosition(collider->getPosition() + dir);
        ff::Vec2f nVel = (collider->getPosition() - collider->getPrevPosition()) / one_frame;
        collider->delta_velocity = nVel - collider->velocity;
        collider->velocity = nVel;

        ground->traverse_add_accel(-500.f);
        ground->traverse_set_speed(-100);
        update();
        render.draw();

        EXPECT_TRUE(box->get_state_flags().has_set(collision_state_t::flags::Wall_L));
    }
}

TEST_F(surfacetracker, on_moving_slope)
{
    initTileMap({
        {"",		"",			"", "", "", "", "", ""},
        {"",	    "",			"", "", "", "", "", ""},
        {"shallow1","shallow2",	"", "", "", "", "", ""},
        {"solid",	"solid",	"", "", "", "", "", ""},
    });
    box->teleport({ 16, 40 });
    box->set_gravity({ 0, 200 });
    box->set_local_vel(Vec2f{1 / one_frame, 0.f});
    ground->settings.surface_friction.kinetic = 100.0f;
    ground->settings.surface_friction.stationary = 100.0f;

    TestPhysRenderer render(world, collider->getBoundingBox());
    render.draw();

    Vec2f dir = Vec2f{ 1.f, 0.f };
    while (render.curr_frame < 60)
    {
        collider->setPosition(collider->getPosition() + dir);
        ff::Vec2f nVel = (collider->getPosition() - collider->getPrevPosition()) / one_frame;
        collider->delta_velocity = nVel - collider->velocity;
        collider->velocity = nVel;

        update();
        render.draw();

        //EXPECT_TRUE(box->get_state_flags().has_set(collision_state_t::flags::Floor));
    }
}
