
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
	bordersCardinalBits(0u)
{

}

Level::Level(GameContext context, const LevelAsset& levelData) :
	context(context),
	objLayer()
{
	init(levelData);
}


void Level::update(secs deltaTime) {
	if (deltaTime == 0.0)
		return;

	//objLayer.update(deltaTime);

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

	for (auto& fg : fgLayers) {
		fg.predraw(deltaTime);
	}

	for (auto& bg : bgLayers) {
		bg.predraw(deltaTime);
	}
}

void Level::init(const LevelAsset& levelData) {
	levelName = levelData.getAssetName();
	bgColor = levelData.getBGColor();
	levelSize = levelData.getTileDimensions();
	bordersCardinalBits = levelData.getBorder();

	fgLayers.clear();
	bgLayers.clear();
	objLayer.clear();

	bool bg = true;
	unsigned bg_count = 0u;
	unsigned fg_count = 0u;
	for (auto& layerRef : *levelData.getLayerRefs()) 
	{
		if (layerRef.type == LayerRef::Type::Tile) {
			(bg ? bg_count++ : fg_count++);
		}
		else {
			bg = false;
		}
	}

	fgLayers.reserve(fg_count);
	bgLayers.reserve(bg_count);

	bg = true;
	for (auto i = levelData.getLayerRefs()->begin(); i != levelData.getLayerRefs()->end(); i++) {
		if (i->type == LayerRef::Type::Object) {
			bg = false;
			objLayer.initFromAsset(context, i->id, i->asObjLayer());
		}
		else if (i->type == LayerRef::Type::Tile) {
			(bg ? bgLayers : fgLayers).push_back(
				TileLayer(context, i->id, i->asTileLayer(), (!bg && fgLayers.empty()) )
			);
		}
	}

	set_borders(bordersCardinalBits);

}


void Level::set_borders(unsigned bordersCardinalBits)
{
	if (!fgLayers.empty()) {
		auto* colMap = fgLayers.front().getCollisionMap();

		if (colMap) {
			colMap->setBorders(levelSize, bordersCardinalBits);
		}
	}
}

void Level::resize(Vec2u n_size)
{
	bordersCardinalBits;
	set_borders(0u);
	for (auto& layer : bgLayers) {
		Vec2u layer_size{
			std::min(n_size.x, layer.getSize().x),
			std::min(n_size.y, layer.getSize().y),
		};

		TileLayer n_layer{ context, layer.getID(), layer_size, layer.hasCollision };
		n_layer.shallow_copy(layer, Rectu{ Vec2u{}, Vec2u{layer_size} }, n_size);
		layer = std::move(n_layer);
	}
	for (auto& layer : fgLayers) {
		Vec2u layer_size{
			std::min(n_size.x, layer.getSize().x),
			std::min(n_size.y, layer.getSize().y),
		};

		TileLayer n_layer{ context, layer.getID(), n_size, layer.hasCollision };
		n_layer.shallow_copy(layer, Rectu{ Vec2u{}, Vec2u{layer_size} }, n_size);
		layer = std::move(n_layer);
	}
	levelSize = n_size;
	set_borders(bordersCardinalBits);
	return;
}

}