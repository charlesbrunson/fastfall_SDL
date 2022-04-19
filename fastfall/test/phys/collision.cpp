
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
};


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

TEST_F(collision, static_tunneling)
{
	initTileMap({
		/*          x:0         x:16		x:32		x:48		x:64 */
		/* y:0 _*/ {"",			"",			"halfvert-h", "",			""},
		/* y:16_*/ {"",			"",			"halfvert-h", "",			""},
		/* y:32_*/ {"",			"",			"halfvert-h", "",			""},
		});

	box->teleport(Vec2f{ 8, 40 });
	box->set_vel(Vec2f{ 128.f, 0.f } / one_frame); // very fast


	TestPhysRenderer render(collider->getBoundingBox());
	render.frame_delay = 20;
	render.draw(colMan);

	while (render.curr_frame < 8) {

		update();
		render.draw(colMan);

		if (render.curr_frame == 1)
		{
			EXPECT_EQ(box->get_contacts().size(), 1);

			if (!box->get_contacts().empty()) {
				const auto& contact = box->get_contacts().at(0);
				EXPECT_TRUE(contact.hasContact);
				EXPECT_TRUE(contact.ortho_normal == Vec2f(-1.f, 0.f));
				EXPECT_TRUE(contact.collider_normal == Vec2f(-1.f, 0.f));
				//EXPECT_EQ(contact.impactTime, 0.5f);

				EXPECT_TRUE(box->getPosition().x == 32.f);
			}
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
	box->set_vel(Vec2f{ 20.f, 0.f } / one_frame);

	TestPhysRenderer render(collider->getBoundingBox());
	render.frame_delay = 20;
	render.draw(colMan);

	while (render.curr_frame < 8) {

		collider->setPosition(collider->getPosition() + Vec2f{ -32.f, 0.f });
		ff::Vec2f nVel = (collider->getPosition() - collider->getPrevPosition()) / one_frame;
		collider->delta_velocity = nVel - collider->velocity;
		collider->velocity = nVel;

		update();
		render.draw(colMan);

		if (render.curr_frame == 3)
		{
			EXPECT_EQ(box->get_contacts().size(), 1);

			if (!box->get_contacts().empty()) {
				const auto& contact = box->get_contacts().at(0);
				EXPECT_TRUE(contact.hasContact);
				EXPECT_TRUE(contact.ortho_normal == Vec2f(-1.f, 0.f));
				EXPECT_TRUE(contact.collider_normal == Vec2f(-1.f, 0.f));
			}
		}
	}
}