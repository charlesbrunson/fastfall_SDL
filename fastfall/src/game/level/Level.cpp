#include "fastfall/game/level/Level.hpp"

#include "fastfall/game/World.hpp"

#include <assert.h>

#include "imgui.h"

namespace ff {

// LEVEL


Level::Level(World& world, ID<Level> t_id)
    : m_id(t_id)
{
}

Level::Level(World& world, ID<Level> t_id, const LevelAsset& levelData)
    : m_id(t_id)
{
    initFromAsset(world, levelData);
}


void Level::clean(World& w) {
    levelName = {};
    bgColor = {};
    levelSize = Vec2u{};
    for(auto& tile_layer : layers.get_tile_layers())
    {
        tile_layer.tilelayer.clean(w);
    }
    layers.clear_all();
}

void Level::update(World& world, secs deltaTime) {
	for (auto& [pos, layer] : layers.get_tile_layers()) {
		layer.update(world, deltaTime);
	}	
}

void Level::predraw(World& world, float interp, bool updated) {

	for (auto& [pos, layer] : layers.get_tile_layers()) {
		layer.predraw(world, interp, updated);
	}
}

void Level::initFromAsset(World& world, const LevelAsset& levelData)
{
    clean(world);

	levelName = levelData.getAssetName();
	bgColor = levelData.getBGColor();
	levelSize = levelData.getTileDimensions();

	for (auto& layerRef : levelData.getLayerRefs().get_tile_layers())
	{
		if (layerRef.position < 0) {
			layers.push_bg_front(TileLayer{ world, m_id, layerRef.tilelayer });
		}
		else {
			layers.push_fg_front(TileLayer{ world, m_id, layerRef.tilelayer });
		}
	}

	for (auto& layer : layers.get_tile_layers())
	{
		layer.tilelayer.set_layer(world, layer.position);
	}

	layers.get_obj_layer().initFromAsset(
		levelData.getLayerRefs().get_obj_layer()
	);
}

void Level::resize(World& world, Vec2u n_size)
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

		TileLayer n_layer{ m_id, layer.getID(), n_size };
		n_layer.set_layer(world, layer.get_layer());
		n_layer.set_collision(world, layer.hasCollision(), layer.getCollisionBorders());
		n_layer.set_scroll(world, layer.hasScrolling(), layer.getScrollRate());
		n_layer.set_parallax(world, layer.hasParallax(), parallax_size);
		n_layer.shallow_copy(world, layer, Rectu{ Vec2u{}, Vec2u{layer_size} }, {0, 0});
		layer = std::move(n_layer);
	}
	levelSize = n_size;
	return;
}

}
