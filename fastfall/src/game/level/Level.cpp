#include "fastfall/game/level/Level.hpp"

#include "fastfall/game/World.hpp"

#include <assert.h>

#include "imgui.h"

namespace ff {

// LEVEL

Level::Level(World* w)
    : world(w)
{

}

Level::Level(World* w, const LevelAsset& levelData)
    : world(w)
{
	init(levelData);
}


void Level::update(secs deltaTime) {
	for (auto& [pos, layer] : layers.get_tile_layers()) {
		layer.update(deltaTime);
	}	
}

void Level::predraw(float interp, bool updated) {

	for (auto& [pos, layer] : layers.get_tile_layers()) {
		layer.predraw(interp, updated);
	}
}

void Level::init(const LevelAsset& levelData) 
{

	asset = &levelData;

	layers.clear_all();

	levelName = levelData.getAssetName();
	bgColor = levelData.getBGColor();
	levelSize = levelData.getTileDimensions();

	for (auto& layerRef : levelData.getLayerRefs().get_tile_layers())
	{
		if (layerRef.position < 0) {
			layers.push_bg_front(TileLayer{ world, layerRef.tilelayer });
		}
		else {
			layers.push_fg_front(TileLayer{ world, layerRef.tilelayer });
		}
	}

	for (auto& layer : layers.get_tile_layers())
	{
		layer.tilelayer.set_layer(layer.position);
	}

	layers.get_obj_layer().initFromAsset(
		levelData.getLayerRefs().get_obj_layer()
	);
}

void Level::resize(Vec2u n_size)
{
	for (auto& [pos, layer] : layers.get_tile_layers())
	{
		Vec2u layer_size{
			std::min(n_size.x, layer.getLevelSize().x),
			std::min(n_size.y, layer.getLevelSize().y)
		};

		Vec2u parallax_size{
			std::min(n_size.x, layer.getParallaxSize().x),
			std::min(n_size.y, layer.getParallaxSize().y)
		};

		TileLayer n_layer{ world, layer.getID(), n_size };
		n_layer.set_layer(layer.get_layer());
		n_layer.set_collision(layer.hasCollision(), layer.getCollisionBorders());
		n_layer.set_scroll(layer.hasScrolling(), layer.getScrollRate());
		n_layer.set_parallax(layer.hasParallax(), parallax_size);
		n_layer.shallow_copy(layer, Rectu{ Vec2u{}, Vec2u{layer_size} }, {0, 0});
		layer = std::move(n_layer);
	}
	levelSize = n_size;
	return;
}

}