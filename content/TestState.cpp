
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

	instance->getActiveLevel()->update(0.0);
	instance->getObject().update(0.0);
	instance->getCollision().update(0.0);

	stateID = ff::EngineStateID::TEST_STATE;

	instance->getScene().set_bg_color(instance->getActiveLevel()->getBGColor());
	instance->getScene().set_size(instance->getActiveLevel()->size());


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
	}

	instance->getActiveLevel()->update(deltaTime);
	instance->getObject().update(deltaTime);
	instance->getTrigger().update(deltaTime);
	instance->getCollision().update(deltaTime);
	instance->getCamera().update(deltaTime);
	

	if (Input::getMouseInView() && Input::isHeld(InputType::MOUSE1)) 
	{
		Level* lvl = instance->getActiveLevel();
		Vec2f mpos = Input::getMouseWorldPosition();
		Vec2u tpos = Vec2u{ mpos / TILESIZE_F };

		if (Rectf{ Vec2f{}, Vec2f{lvl->size()} *TILESIZE_F }.contains(mpos) && (!painting ||  last_paint != tpos)) {

			LevelEditor edit{ lvl };
			edit.select_layer(1);
			edit.select_tileset("tile_test");
			edit.select_tile(Vec2u{});
			edit.paint_tile(tpos);
		}
		last_paint = tpos;
		painting = true;
	}
	else if (Input::getMouseInView() && Input::isHeld(InputType::MOUSE2))
	{
		Level* lvl = instance->getActiveLevel();
		Vec2f mpos = Input::getMouseWorldPosition();
		Vec2u tpos = Vec2u{ mpos / TILESIZE_F };

		if (Rectf{ Vec2f{}, Vec2f{lvl->size()} *TILESIZE_F }.contains(mpos) && (!painting || last_paint != tpos)) {

			LevelEditor edit{ lvl };
			edit.select_layer(1);
			edit.select_tileset("tile_test");
			edit.select_tile(Vec2u{});
			edit.erase_tile(tpos);
		}
		last_paint = tpos;
		painting = true;
	}
	else {
		painting = false;
	}

}

void TestState::predraw(secs deltaTime) {
	if (instance->want_reset)
		instance->reset();

	instance->getObject().predraw(deltaTime);
	instance->getActiveLevel()->predraw(deltaTime);
	viewPos = instance->getCamera().currentPosition;
	viewZoom = instance->getCamera().zoomFactor;

	instance->getScene().set_cam_pos(viewPos);
}

void TestState::draw(ff::RenderTarget& target, ff::RenderState state) const {

	target.draw(instance->getScene(), state);

}