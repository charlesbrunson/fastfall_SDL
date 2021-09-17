
#include "fastfall/game/Level.hpp"
#include <assert.h>

//#include "level/LevelObserver.hpp"
#include "imgui.h"

#include "fastfall/game/Instance.hpp"
#include "fastfall/game/GameObjectManager.hpp"

namespace ff {

Level::Level(GameContext context) :
	context(context),
	objLayer()
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

	for (auto& [pos, layer] : layers) {
		layer.update(deltaTime);
	}	
}

void Level::predraw(secs deltaTime) {

	for (auto& [pos, layer] : layers) {
		layer.predraw(deltaTime);
	}
}

void Level::init(const LevelAsset& levelData) {
	levelName = levelData.getAssetName();
	bgColor = levelData.getBGColor();
	levelSize = levelData.getTileDimensions();

	layers.clear();
	objLayer.clear();
	fg1_layer_ndx = 0;

	// count the layers first
	bool bg = true;
	int bg_count = 0u;
	int fg_count = 0u;
	for (auto& layerRef : *levelData.getLayerRefs())
	{
		switch (layerRef.type)
		{
		case LayerRef::Type::Object: 
			bg = false; 
			break;
		case LayerRef::Type::Tile: 
			(bg ? bg_count : fg_count)++;
			break;
		}
	}

	layers.reserve((size_t)bg_count + fg_count);
	//fg1_layer_ndx = bg_count;

	// start building the layers
	int count = 0;
	for (auto& layerRef : *levelData.getLayerRefs())
	{
		switch (layerRef.type)
		{
		case LayerRef::Type::Object:
			objLayer.initFromAsset(context, layerRef.id, layerRef.asObjLayer());
			break;
		case LayerRef::Type::Tile:
			bool is_bg = count - bg_count < 0;
			insertTileLayer({
					.position = (is_bg ? -1 : count - bg_count),
					.tilelayer = TileLayer(context, layerRef.id, layerRef.asTileLayer())
				});
			count++;
			break;
		}
	}
}


LevelLayer& Level::insertTileLayer(LevelLayer&& layer)
{
	std::vector<LevelLayer>::iterator ret;
	if (layer.position < 0) {
		auto it = std::upper_bound(
			layers.begin(), layers.end(),
			layer.position,
			[](int pos, const LevelLayer& lvllayer) {
				return lvllayer.position > pos;
			});

		ret = layers.insert(it, std::move(layer));

		fg1_layer_ndx++;
		for (int i = -fg1_layer_ndx; i < 0; i++) {
			layers.at(i + fg1_layer_ndx).position = i;
		}
	}
	else {
		auto it = std::upper_bound(
			layers.begin(), layers.end(),
			layer.position,
			[](int pos, const LevelLayer& lvllayer) {
				return lvllayer.position > pos;
			});

		ret = layers.insert(it, std::move(layer));
		for (int i = 0; i < (layers.size() - fg1_layer_ndx); i++) {
			layers.at(i + fg1_layer_ndx).position = i;
		}
	}
	return *ret;
}
void Level::removeTileLayer(int position)
{
	auto it = std::find_if(
		layers.begin(), layers.end(),
		[position](const LevelLayer& lvllayer) {
			return lvllayer.position == position;
		});

	if (it != layers.end())
	{
		layers.erase(it);
		if (position < 0) {
			fg1_layer_ndx--;
		}

		for (int i = 0; i < layers.size(); i++) 
		{
			layers.at(i).position = i - fg1_layer_ndx;
		}
	}
}


void Level::resize(Vec2u n_size)
{
	for (auto& [pos, layer] : layers)
	{
		Vec2u layer_size{
			std::min(n_size.x, layer.getSize().x),
			std::min(n_size.y, layer.getSize().y)
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
