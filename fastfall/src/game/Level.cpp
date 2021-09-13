
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
	bordersCardinalBits = levelData.getBorder();

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
			using enum LayerRef::Type;
		case Object: 
			bg = false; 
			break;
		case Tile: 
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
			using enum LayerRef::Type;
		case Object:
			objLayer.initFromAsset(context, layerRef.id, layerRef.asObjLayer());
			break;
		case Tile:
			/*
			layers.push_back({
					.position = count - bg_count,
					.tilelayer = TileLayer(context, layerRef.id, layerRef.asTileLayer(), (count - bg_count == 0))
				});
			*/
			bool is_bg = count - bg_count < 0;
			bool has_collision = (count - bg_count == 0);
			insertTileLayer({
					.position = (is_bg ? -1 : count - bg_count),
					.tilelayer = TileLayer(context, layerRef.id, layerRef.asTileLayer(), has_collision)
				});
			count++;
			break;
		}
	}

	// apply borders
	if (bordersCardinalBits) {
		set_borders(bordersCardinalBits);
	}
}


void Level::insertTileLayer(LevelLayer&& layer)
{
	if (layer.position < 0) {
		auto it = std::upper_bound(
			layers.begin(), layers.end(),
			layer.position,
			[](int pos, const LevelLayer& lvllayer) {
				return lvllayer.position > pos;
			});

		layers.insert(it, std::move(layer));

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

		layers.insert(it, std::move(layer));
		for (int i = 0; i < (layers.size() - fg1_layer_ndx); i++) {
			layers.at(i + fg1_layer_ndx).position = i;

			if (i == 0 && !layers.at(i + fg1_layer_ndx).tilelayer.has_collision())
			{
				layers.at(i + fg1_layer_ndx).tilelayer.enable_collision();
			}
			else if (i > 0 && layers.at(i + fg1_layer_ndx).tilelayer.has_collision())
			{
				layers.at(i + fg1_layer_ndx).tilelayer.remove_collision();
			}
		}
	}
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
			fg1_layer_ndx;
		}

		for (int i = 0; i < layers.size(); i++) 
		{
			layers.at(i + fg1_layer_ndx).position = i - fg1_layer_ndx;
		}

		if (layers.size() > fg1_layer_ndx && !layers.at(0).tilelayer.has_collision()) 
		{
			layers.at(fg1_layer_ndx).tilelayer.enable_collision();
		}
	}
}

void Level::set_borders(unsigned bordersCardinalBits)
{
	if (layers.size() > fg1_layer_ndx) 
	{
		if (auto* colMap = layers.at(fg1_layer_ndx).tilelayer.getCollisionMap()) 
		{
			colMap->setBorders(levelSize, bordersCardinalBits);
		}
	}
}

void Level::resize(Vec2u n_size)
{
	bordersCardinalBits;
	set_borders(0u);

	for (auto& [pos, layer] : layers)
	{

		Vec2u layer_size{
			std::min(n_size.x, layer.getSize().x),
			std::min(n_size.y, layer.getSize().y),
		};

		TileLayer n_layer{ context, layer.getID(), (pos < 0 ? layer_size : n_size), layer.has_collision() };
		n_layer.shallow_copy(layer, Rectu{ Vec2u{}, Vec2u{layer_size} }, n_size);
		layer = std::move(n_layer);

	}

	levelSize = n_size;
	set_borders(bordersCardinalBits);
	return;
}

}