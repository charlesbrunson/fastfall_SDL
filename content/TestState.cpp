
#include "TestState.hpp"

#include "fastfall/game/Instance.hpp"
#include "fastfall/render/ShapeRectangle.hpp"
#include "fastfall/render/ShapeCircle.hpp"
#include "fastfall/render/ShapeLine.hpp"
#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/engine/input.hpp"

#include "tilelogic/AnimLogic.hpp"

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

			//if (!lvl->getFGLayers().empty()) {



				//instance->getCollision().addColliderRegion(lvl->getFGLayers().begin()->getCollisionMap());
			//}

			float xf = static_cast<float>(lvl->size().x);
			float yf = static_cast<float>(lvl->size().y);
			viewPos = ff::Vec2f(xf, yf) * TILESIZE_F / 2.f;

			//clearColor = lvl->getBGColor();
			clearColor = ff::Color{ 0x141013FF };
		}
	}


	instance->getActiveLevel()->update(0.0);
	instance->getObject().update(0.0);
	instance->getCollision().update(0.0);

	stateID = ff::EngineStateID::TEST_STATE;


}

TestState::~TestState() {
	DestroyInstance(instance->getInstanceID());
}

void TestState::update(secs deltaTime) {

	if (deltaTime > 0.0) {
		auto* tile = &*instance->getActiveLevel()->getFGLayers().begin();
		auto colmap = tile->getCollisionMap();
		
		static secs timebuf = 0.0;
		timebuf += deltaTime;

		colmap->setPosition(colmap->getPosition());
	}

	instance->getActiveLevel()->update(deltaTime);
	instance->getObject().update(deltaTime);
	instance->getTrigger().update(deltaTime);
	instance->getCollision().update(deltaTime);
	instance->getCamera().update(deltaTime);
	

}

void TestState::predraw(secs deltaTime) {
	instance->getObject().predraw(deltaTime);
	//instance->getCollision().predraw(deltaTime);
	instance->getActiveLevel()->predraw(deltaTime);
	viewPos = instance->getCamera().currentPosition;
	viewZoom = instance->getCamera().zoomFactor;

	instance->getScene().set_bg_color(instance->getActiveLevel()->getBGColor());
	instance->getScene().set_size(instance->getActiveLevel()->size());
}

void TestState::draw(ff::RenderTarget& target, ff::RenderState state) const {

	target.draw(instance->getScene(), state);

	/*
	if (instance->enableScissor(target, viewPos)) {

		Vec2f size = Vec2f{ instance->getActiveLevel()->size() } *TILESIZE_F;

		
		ff::ShapeRectangle bgRect(
			ff::Rectf(ff::Vec2f{0.f, 0.f}, ff::Vec2f(size)),
			instance->getActiveLevel()->getBGColor()
		);
		target.draw(bgRect, state);
		

		for (auto& bg : instance->getActiveLevel()->getBGLayers()) {
			target.draw(bg, state);
		}
		
	}
	instance->disableScissor();

	target.draw(instance->getObject().getObjectDrawList(false), state);

	bool firstFG = true;
	for (auto& fg : instance->getActiveLevel()->getFGLayers()) {

		target.draw(fg, state);

		if (firstFG) {
			firstFG = false;
			target.draw(instance->getObject().getObjectDrawList(true), state);
		}
	}

	*/
}