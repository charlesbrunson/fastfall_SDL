
#include "fastfall/game/Level.hpp"
#include <assert.h>

//#include "level/LevelObserver.hpp"
#include "imgui.h"

#include "fastfall/game/Instance.hpp"
#include "fastfall/game/GameObjectManager.hpp"

namespace ff {

// LEVEL

Level::Level(GameContext context) :
	context(context)
{

}

Level::Level(GameContext context, const LevelAsset& levelData) :
	context(context)
{
	init(levelData);
}


void Level::update(secs deltaTime) {
	if (deltaTime == 0.0)
		return;

	for (auto& [pos, layer] : layers.get_tile_layers()) {
		layer.update(deltaTime);
	}	
}

void Level::predraw(secs deltaTime) {

	for (auto& [pos, layer] : layers.get_tile_layers()) {
		layer.predraw(deltaTime);
	}
}

void Level::init(const LevelAsset& levelData) 
{
	layers.clear_all();

	levelName = levelData.getAssetName();
	bgColor = levelData.getBGColor();
	levelSize = levelData.getTileDimensions();

	for (auto& layerRef : levelData.getLayerRefs().get_tile_layers())
	{
		if (layerRef.position < 0) {
			layers.push_bg_front(TileLayer{ context, layerRef.tilelayer });
		}
		else {
			layers.push_fg_front(TileLayer{ context, layerRef.tilelayer });
		}
	}
	layers.get_obj_layer().initFromAsset(
		context,
		levelData.getLayerRefs().get_obj_layer()
	);
}

void Level::resize(Vec2u n_size)
{
	for (auto& [pos, layer] : layers.get_tile_layers())
	{
		Vec2u layer_size{
			std::min(n_size.x, layer.get_level_size().x),
			std::min(n_size.y, layer.get_level_size().y)
		};

		Vec2u parallax_size{
			std::min(n_size.x, layer.get_parallax_size().x),
			std::min(n_size.y, layer.get_parallax_size().y)
		};

		TileLayer n_layer{ context, layer.getID(), n_size };
		n_layer.set_collision(layer.has_collision(), layer.get_collision_border());
		n_layer.set_scroll(layer.has_scroll(), layer.get_scrollrate());
		n_layer.set_parallax(layer.has_parallax(), parallax_size);
		n_layer.shallow_copy(layer, Rectu{ Vec2u{}, Vec2u{layer_size} }, n_size);
		layer = std::move(n_layer);
	}
	levelSize = n_size;
	return;
}

}
