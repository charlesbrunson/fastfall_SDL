#include "fastfall/game/level/LevelEditor.hpp"

#include "fastfall/resource/Resources.hpp"

#include <chrono>

namespace ff {


LevelEditor::LevelEditor(Level& lvl, bool show_imgui) 
{
	level = &lvl;
	assert(level);

}

LevelEditor::LevelEditor(GameContext context, bool show_imgui, std::string name, Vec2u tile_size)
{
	assert(tile_size.x >= LevelEditor::MIN_LEVEL_SIZE.x);
	assert(tile_size.y >= LevelEditor::MIN_LEVEL_SIZE.y);

	created_level = std::make_unique<Level>(context);
	level = created_level.get();

	assert(level);
}


// LAYERS

// create layer at position, selects it
bool LevelEditor::create_layer(int layer_pos)
{
	if (!level) return false;

	int bg_count = level->get_layers().get_bg_count();
	int fg_count = level->get_layers().get_fg_count();

	if (layer_pos != Level::Layers::OBJECT_LAYER_POS
		&& layer_pos >= -bg_count - 1
		&& layer_pos <= fg_count + 1)
	{
		level->get_layers().insert(
			layer_pos,
			TileLayer{ level->getContext(), 0, level->size() }
		);
	}
	return false;
}

// select layer at positon (start and end specify the first and last layer, respectively)
bool LevelEditor::select_layer(int layer_pos)
{
	if (!level) return false;

	int bg_count = level->get_layers().get_bg_count();
	int fg_count = level->get_layers().get_fg_count();

	if (layer_pos != Level::Layers::OBJECT_LAYER_POS) {
		curr_layer = level->get_layers().get_tile_layer_at(layer_pos);
		obj_layer_selected = false;
	}
	else {
		obj_layer_selected = true;
	}

	if (!curr_layer) 
	{
		deselect_layer();
	}
	return curr_layer || obj_layer_selected;
}

void LevelEditor::select_obj_layer()
{
	obj_layer_selected = true;
	curr_layer = nullptr;
}

void LevelEditor::deselect_layer()
{
	obj_layer_selected = false;
	curr_layer = nullptr;
}

// move selected layer to new position
// retains selection of moved layer
bool LevelEditor::move_layer(int layer_pos)
{

	if (!level) return false;


	if (obj_layer_selected)
	{
		// ...
	}
	else if (curr_layer)
	{
		int curr_pos = curr_layer->position;
		while (curr_pos != layer_pos) {

			int n_pos = 0;
			while (curr_pos > layer_pos)
			{
				n_pos = level->get_layers().swap_prev(curr_pos);
				if (n_pos == curr_pos)
					break;

				curr_pos = n_pos;
			}
			while (curr_pos < layer_pos)
			{
				n_pos = level->get_layers().swap_next(curr_pos);
				if (n_pos == curr_pos)
					break;

				curr_pos = n_pos;
			}
		}
		return curr_pos == layer_pos;
	}
	return false;
}

// erases selected layer
// deselects layer
bool LevelEditor::erase_layer()
{
	if (level && curr_layer)
	{
		level->get_layers().erase(curr_layer->position);	
	}
	return false;
}

// LAYER PROPERTIES

bool LevelEditor::layer_set_collision(bool enabled, unsigned borderBits)
{
	if (level && curr_layer) {
		return curr_layer->tilelayer.set_collision(enabled, borderBits);
	}
	return false;
}
bool LevelEditor::layer_set_scroll(bool enabled, Vec2f scroll_rate)
{
	if (level && curr_layer) {
		return curr_layer->tilelayer.set_scroll(enabled, scroll_rate);
	}
	return false;
}
bool LevelEditor::layer_set_parallax(bool enabled, Vec2u parallax_size)
{
	if (level && curr_layer) {
		return curr_layer->tilelayer.set_parallax(enabled, parallax_size);
	}
	return false;
}

// TILES

// paints tile onto selected layer, using selected tileset and tile
bool LevelEditor::paint_tile(Vec2u pos)
{
	if (!level) return false;

	if (curr_layer && curr_tileset && tileset_pos) {
		Vec2u size = curr_layer->tilelayer.getLevelSize();

		if (pos.x < size.x && pos.y < size.y) 
		{
			curr_layer->tilelayer.setTile(pos, *tileset_pos, *curr_tileset);
			return true;

		}
	}
	return false;
}

// paints tile onto selected layer
bool LevelEditor::erase_tile(Vec2u pos)
{
	if (!level) return false;

	if (curr_layer) {
		Vec2u size = curr_layer->tilelayer.getLevelSize();

		if (pos.x < size.x && pos.y < size.y)
		{
			curr_layer->tilelayer.removeTile(pos);
			return true;
		}
	}
	return false;
}

// TILESET

// selects tileset for painting tiles
bool LevelEditor::select_tileset(std::string_view tileset_name)
{
	if (!level) return false;

	if (!curr_tileset || curr_tileset->getAssetName() != tileset_name) {
		curr_tileset = Resources::get<TilesetAsset>(tileset_name);
		deselect_tile();
	}
	return curr_tileset != nullptr;
}

void LevelEditor::deselect_tileset()
{
	curr_tileset = nullptr;
	deselect_tile();
}

// selects tile from selected tileset for painting tiles
bool LevelEditor::select_tile(TileID tile)
{
	if (!level) return false;

	if (curr_tileset 
		&& tile.x < curr_tileset->getTileSize().x
		&& tile.y < curr_tileset->getTileSize().y)
	{
		tileset_pos = tile;
		return true;
	}
	return false;
}

void LevelEditor::deselect_tile()
{
	tileset_pos = std::nullopt;
}

// LEVEL PROPERTIES

// changes level's name
bool LevelEditor::set_name(std::string name)
{
	if (!level) return false;

	level->set_name(name);
	return true;
}

// changes level's background color
bool LevelEditor::set_bg_color(Color bg_color)
{
	if (!level) return false;

	level->set_bg_color(bg_color);
	return true;
}


bool LevelEditor::set_size(Vec2u size)
{
	if (level && level->size() != size) {
		level->resize(size);
		return true;
	}
	return false;
}


bool LevelEditor::applyLevelAsset(const LevelAsset* asset)
{
	if (!level) return false;

	auto start = std::chrono::system_clock::now();

	// step 1: level properties
	if (level->name()		!= asset->getAssetName())		set_name(asset->getAssetName());
	if (level->size()		!= asset->getTileDimensions())	set_size(asset->getTileDimensions());
	if (level->getBGColor() != asset->getBGColor())			set_bg_color(asset->getBGColor());

	// step 2: correct layer ordering

	auto asset_ids = asset->getLayerRefs().get_id_order();

	// erase ids not present in asset_ids
	auto& lvl_layers = level->get_layers();

	auto erase_layer = [&](int i) -> bool
	{
		int id = lvl_layers.get_tile_layer_at(i)->tilelayer.getID();
		auto asset_it = std::find(asset_ids.begin(), asset_ids.end(), id);

		if (asset_it == asset_ids.end())
		{
			lvl_layers.erase(i);
		}
		return asset_it == asset_ids.end();
	};

	for (int i = 1; i <= lvl_layers.get_fg_count(); i++)
	{
		if (erase_layer(i)) i--;
	}
	for (int i = -1; i >= -lvl_layers.get_bg_count(); i--)
	{
		if (erase_layer(i)) i++;
	}

	// create layers not present in level
	std::set<unsigned> nLayers;
	for (auto layer_ref : asset->getLayerRefs().get_tile_layers()) 
	{
		bool not_in_lvl = std::none_of(
			lvl_layers.get_tile_layers().begin(),
			lvl_layers.get_tile_layers().end(),
			[&layer_ref](const Level::Layers::TileEntry& layer) {
				return layer_ref.tilelayer.getID() == layer.tilelayer.getID();
			}
		);
		if (not_in_lvl)
		{
			lvl_layers.push_fg_front(TileLayer{ level->getContext(), layer_ref.tilelayer });
			nLayers.insert(layer_ref.tilelayer.getID());
		}
	}

	// reorder layers
	lvl_layers.reorder(asset_ids);

	// step 3: per layer update
	auto it = asset->getLayerRefs().get_tile_layers().begin();
	for (auto& layer : lvl_layers.get_tile_layers())
	{
		if (nLayers.contains(layer.tilelayer.getID())) {
			// this is a created layer, no need to update
			it++;
			continue;
		}

		assert(layer.tilelayer.getID() == it->tilelayer.getID());

		const TileLayerData& tile_ref = it->tilelayer;


		select_layer(layer.position);

		// disable layer properties
		if (layer.tilelayer.hasCollision() && !tile_ref.hasCollision())
		{
			layer.tilelayer.set_collision(false);
			LOG_INFO("disable collision");
		}
		if (layer.tilelayer.hasScrolling() && !tile_ref.hasScrolling())
		{
			layer.tilelayer.set_scroll(false);
			LOG_INFO("disable scroll");
		}
		if (layer.tilelayer.hasParallax() && !tile_ref.hasParallax())
		{
			layer.tilelayer.set_parallax(false);
			LOG_INFO("disable parallax");
		}

		// enable or update layer properties
		if ((!layer.tilelayer.hasCollision() && tile_ref.hasCollision()) ||
			(tile_ref.hasCollision() && (layer.tilelayer.getCollisionBorders() != tile_ref.getCollisionBorders())))
		{
			layer.tilelayer.set_collision(true, tile_ref.getCollisionBorders());
			LOG_INFO("enable collision");
		}
		if ((!layer.tilelayer.hasScrolling() && tile_ref.hasScrolling()) ||
			(tile_ref.hasScrolling() && (layer.tilelayer.getScrollRate() != tile_ref.getScrollRate())))
		{
			layer.tilelayer.set_scroll(true, tile_ref.getScrollRate());
			LOG_INFO("enable scroll");
		}
		if ((!layer.tilelayer.hasParallax() && tile_ref.hasParallax()) ||
			(tile_ref.hasParallax() && (layer.tilelayer.getParallaxSize() != tile_ref.getParallaxSize())))
		{
			layer.tilelayer.set_parallax(true, tile_ref.getParallaxSize());
			LOG_INFO("enable parallax");
		}

		// update tiles
		//unsigned tile_ndx = 0;

		unsigned width = tile_ref.getSize().x;
		unsigned height = tile_ref.getSize().y;

		unsigned paint_count = 0;
		unsigned erase_count = 0;

		const auto& tile_data = tile_ref.getTileData();
		for (unsigned yy = 0; yy < height; yy++) {
			for (unsigned xx = 0; xx < width; xx++) {

				Vec2u pos{ xx, yy };
				//unsigned ndx = xx + yy * tile_ref.getSize().x;

				//if (tile_ndx >= tile_ref.getSize().x * tile_ref.getSize().x)
				//	break;

				auto& tilelayer = layer.tilelayer;

				if (tile_data[pos].has_tile)
				{
					auto tile_id = tile_data[pos].tile_id;
					auto tileset = tile_ref.getTilesetFromNdx(tile_data[pos].tileset_ndx);

					if ( (!tilelayer.hasTileAt(pos))
						|| (tilelayer.getTileID(pos).value() != tile_id)
						|| (tilelayer.getTileTileset(pos)->getAssetName() != *tileset))
					{
						paint_count++;
						select_tileset(tileset->c_str());
						select_tile(tile_id);
						paint_tile(pos);
					}
				}
				else if (tilelayer.hasTileAt(pos)) {
					erase_count++;
					erase_tile(pos);
				}
			}
		}


		// predraw to apply changes
		layer.tilelayer.predraw(0.0);

		// increment to next tilelayer ref
		it++; 

		LOG_INFO("updated layer #{}: {} tile affected", layer.tilelayer.getID(), paint_count + erase_count);
	}


	// step 4: check objects



	std::chrono::duration<double> duration = std::chrono::system_clock::now() - start;
	LOG_INFO("apply duration: {}s", duration.count());


	return true;
}

}
