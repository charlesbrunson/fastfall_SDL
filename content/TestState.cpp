
#include "TestState.hpp"

#include "fastfall/game/Instance.hpp"
#include "fastfall/render/ShapeRectangle.hpp"
#include "fastfall/render/ShapeCircle.hpp"
#include "fastfall/render/ShapeLine.hpp"
#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/engine/input.hpp"

#include "tilelogic/AnimLogic.hpp"

#include "fastfall/game/level/LevelEditor.hpp"

TestState::TestState()
{


	instance = ff::CreateInstance();
	instanceID = instance->getInstanceID();
	assert(instance);

	ff::LevelAsset* lvlptr = ff::Resources::get<ff::LevelAsset>("map_test");
	assert(lvlptr);
	if (lvlptr) {
		instance->addLevel(*lvlptr);

		ff::Level* lvl = instance->getActiveLevel();
		assert(lvl);
		if (lvl) {

			float xf = static_cast<float>(lvl->size().x);
			float yf = static_cast<float>(lvl->size().y);
			viewPos = ff::Vec2f(xf, yf) * TILESIZE_F / 2.f;

			clearColor = ff::Color{ 0x141013FF };
		}
	}


	//instance->getActiveLevel()->resize(Vec2u{ 240, 60 });
	//instance->populateSceneFromLevel(*instance->getActiveLevel());
	instance->getActiveLevel()->update(0.0);
	instance->getObject().update(0.0);
	instance->getCollision().update(0.0);

	stateID = ff::EngineStateID::TEST_STATE;

	instance->getScene().set_bg_color(instance->getActiveLevel()->getBGColor());
	instance->getScene().set_size(instance->getActiveLevel()->size());


}

void paint(Vec2u start, LevelEditor& edit) {

	unsigned x_off = 0u;

	unsigned letters[] = {
		0b101101111101101,
		0b111100110100111,
		0b100100100100111,
		0b100100100100111,
		0b111101101101111,
		0b000000000000000,
		0b101101111111101,
		0b111101101101111,
		0b111101110101101,
		0b100100100100111,
		0b111101101101111,
	};

	unsigned letter_w = 3u;
	unsigned letter_h = 5u;


	for (unsigned str : letters)
	{
		for (unsigned y = 0; y < letter_h; y++) {
			for (unsigned x = 0; x < letter_w; x++) {
				bool paint = str & ((1 << 14) >> (x + y * letter_w));

				Vec2u pos{ start };
				pos.x += x_off + x;
				pos.y += y;

				if (paint) {
					edit.paint_tile(pos);
				}
				else {
					edit.erase_tile(pos);
				}
			}
		}
		x_off += 4;
	}
}


TestState::~TestState() {
	ff::DestroyInstance(instance->getInstanceID());
}

void TestState::update(secs deltaTime) {

	if (deltaTime > 0.0) {
		auto* tile = &*instance->getActiveLevel()->getFGLayers().begin();
		auto colmap = tile->getCollisionMap();
		
		static secs timebuf = 0.0;
		timebuf += deltaTime;

		//colmap->setPosition(colmap->getPosition());
	}



	instance->getActiveLevel()->update(deltaTime);
	instance->getObject().update(deltaTime);
	instance->getTrigger().update(deltaTime);
	instance->getCollision().update(deltaTime);
	instance->getCamera().update(deltaTime);
	
}

void TestState::predraw(secs deltaTime) {
	//std::scoped_lock lock(instance->reset_mutex);

	if (instance->want_reset)
		instance->reset();

	instance->getObject().predraw(deltaTime);
	instance->getActiveLevel()->predraw(deltaTime);
	viewPos = instance->getCamera().currentPosition;
	viewZoom = instance->getCamera().zoomFactor;

	static secs t_buff = 0.0;
	t_buff += deltaTime;

	if (t_buff > 5.0)
	{
		LevelEditor edit{ instance->getActiveLevel() };
		edit.select_tileset("tile_test");
		edit.select_tile(Vec2u{ 0u, 0u });
		edit.select_layer(1);

		paint(Vec2u{ 6u, 4u }, edit);
	}


	instance->getScene().set_cam_pos(viewPos);
}

void TestState::draw(ff::RenderTarget& target, ff::RenderState state) const {

	target.draw(instance->getScene(), state);

}