
#include "fastfall/game/Level.hpp"
#include <assert.h>

//#include "level/LevelObserver.hpp"
#include "imgui.h"

#include "fastfall/game/Instance.hpp"
#include "fastfall/game/GameObjectManager.hpp"

namespace ff {

Level::Level(GameContext context) :
	context(context),
	objLayer(),
	levelName(nullptr),
	bordersCardinalBits(0u)
{
	//LevelObserver::registerLevel(this);
}

Level::Level(GameContext context, const LevelAsset& levelData) :
	context(context),
	objLayer(),
	levelName(nullptr)
{
	//LevelObserver::registerLevel(this);
	init(levelData);
}

Level::~Level() {
	//LevelObserver::unregisterLevel(this);
}

void Level::update(secs deltaTime) {
	if (deltaTime == 0.0)
		return;

	objLayer.update(deltaTime);

	for (auto& fg : fgLayers) {
		fg.update(deltaTime);
	}

	for (auto& bg : bgLayers) {
		bg.update(deltaTime);
	}

#ifdef DEBUG
	if (fgLayers.begin()->hasCollision) {
		fgLayers.begin()->getCollisionMap()->update(deltaTime);
	}
#endif
}

void Level::predraw(secs deltaTime) {
	//objLayer.getObjMan().predraw(deltaTime);

	for (auto& fg : fgLayers) {
		fg.predraw(deltaTime);
	}

	for (auto& bg : bgLayers) {
		bg.predraw(deltaTime);
	}
}

void Level::draw(RenderTarget& target, RenderState states) const {

	for (auto it = bgLayers.begin(); it != bgLayers.end(); it++) {
		target.draw(*it, states);
	}

	// draw bg objs
	//auto objMan = &Instance(instance)->getObject();
	//target.draw(objMan->getObjectDrawList(), states);

	for (auto it = fgLayers.begin(); it != fgLayers.end(); it++) {

		target.draw(*it, states);
		if (it == fgLayers.begin()) {
			// draw fg objs
			//target.draw(objMan->getObjectDrawList(), states);

		}
	}
}

void Level::init(const LevelAsset& levelData) {
	levelName = &levelData.getAssetName();

	bgColor = levelData.getBGColor();
	levelSize = levelData.getTileDimensions();
	bordersCardinalBits = levelData.getBorder();

	fgLayers.clear();
	bgLayers.clear();
	objLayer.clear();

	bool bg = true;
	for (auto i = levelData.getLayerRefs()->begin(); i != levelData.getLayerRefs()->end(); i++) {
		if (i->type == LayerType::OBJECTLAYER) {
			bg = false;
			objLayer.initFromAsset(context, *i);
		}
		else if (i->type == LayerType::TILELAYER) {
			(bg ? bgLayers : fgLayers).push_back(
				TileLayer(*i, (!bg && fgLayers.empty()))
			);
		}
	}

	if (!fgLayers.empty()) {
		auto* colMap = fgLayers.front().getCollisionMap().get();

		if (colMap) {

			colMap->setBorders(fgLayers.front().getSize(), bordersCardinalBits);
			//colMap->applyChanges();
		}

	}
}

}