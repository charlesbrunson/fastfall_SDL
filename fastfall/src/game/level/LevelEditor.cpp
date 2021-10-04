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

	if (layer_pos != LevelLayerContainer::OBJECT_LAYER_POS
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

	if (layer_pos != LevelLayerContainer::OBJECT_LAYER_POS) {
		curr_layer = level->get_layers().tile_layer_at(layer_pos);
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
		Vec2u size = curr_layer->tilelayer.get_level_size();

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
		Vec2u size = curr_layer->tilelayer.get_level_size();

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
bool LevelEditor::select_tile(Vec2u tile_pos)
{
	if (!level) return false;

	if (curr_tileset 
		&& tile_pos.x < curr_tileset->getTileSize().x
		&& tile_pos.y < curr_tileset->getTileSize().y)
	{
		tileset_pos = tile_pos;
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
	if (level->name() != asset->getAssetName())
	{
		set_name(asset->getAssetName());
	}

	if (level->size() != asset->getTileDimensions())
	{
		set_size(asset->getTileDimensions());
	}

	if (level->getBGColor() != asset->getBGColor())
	{
		set_bg_color(asset->getBGColor());
	}

	// step 2: correct layer ordering

	// get the layer ids & order from the asset
	std::vector<int> asset_ids;
	std::transform(asset->getLayerRefs()->begin(), asset->getLayerRefs()->end(),
		std::back_inserter(asset_ids),
		[](const LayerRef& layer) -> unsigned {
			return layer.type == LayerRef::Type::Tile ? layer.id : 0; // mark obj layer as zero
		});


	// erase ids not present in asset_ids
	for (int i = 1; i <= level->get_layers().get_fg_count(); i++)
	{
		int id = level->get_layers().tile_layer_at(i)->tilelayer.getID();

		auto asset_it = std::find(asset_ids.begin(), asset_ids.end(), id);
		if (asset_it == asset_ids.end())
		{
			level->get_layers().erase(i);
			i--;
		}
	}
	for (int i = -1; i >= -level->get_layers().get_bg_count(); i--)
	{
		int id = level->get_layers().tile_layer_at(i)->tilelayer.getID();

		auto asset_it = std::find(asset_ids.begin(), asset_ids.end(), id);
		if (asset_it == asset_ids.end())
		{
			level->get_layers().erase(i);
			i++;
		}
	}

	// create layers not present in level
	std::set<unsigned> nLayers;
	if (asset_ids.size() > level->get_layers().get_tile_layers().size()) 
	{
		for (auto& layer_ref : *asset->getLayerRefs()) 
		{
			if (layer_ref.type == LayerRef::Type::Object)
				continue;

			bool exists_in_level = std::any_of(
				level->get_layers().get_tile_layers().begin(),
				level->get_layers().get_tile_layers().end(),
				[&layer_ref](const LevelTileLayer& layer) {
					return layer_ref.id == layer.tilelayer.getID();
				}
			);

			if (!exists_in_level)
			{
				level->get_layers().push_fg_front(TileLayer{
						level->getContext(),
						layer_ref.id,
						layer_ref.asTileLayer()
					});
				nLayers.insert(layer_ref.id);
			}
		}
	}

	// reorder layers
	level->get_layers().reorder(asset_ids);

	// step 3: per layer update
	auto it = asset->getLayerRefs()->begin();

	auto& tile_layers = level->get_layers().get_tile_layers();
	for (auto& layer : tile_layers)
	{
		if (nLayers.contains(layer.tilelayer.getID())) {
			// this is a created layer, no need to update
			it++;
			continue;
		}

		if (it->type == LayerRef::Type::Object)
			it++;

		assert(layer.tilelayer.getID() == it->id);

		const TileLayerRef& tile_ref = it->asTileLayer();

		unsigned tile_ndx = 0;
		unsigned width    = tile_ref.tileSize.x;
		unsigned height   = tile_ref.tileSize.y;

		unsigned paint_count = 0;
		unsigned erase_count = 0;

		select_layer(layer.position);

		// disable layer properties
		if (layer.tilelayer.has_collision() && !tile_ref.has_collision)
		{
			layer.tilelayer.set_collision(false);
			LOG_INFO("disable collision");
		}
		if (layer.tilelayer.has_scroll() && !tile_ref.has_scroll)
		{
			layer.tilelayer.set_scroll(false);
			LOG_INFO("disable scroll");
		}
		if (layer.tilelayer.has_parallax() && !tile_ref.has_scroll)
		{
			layer.tilelayer.set_parallax(false);
			LOG_INFO("disable parallax");
		}

		// enable or update layer properties
		if ((!layer.tilelayer.has_collision() && tile_ref.has_collision) ||
			(tile_ref.has_collision && (layer.tilelayer.get_collision_border() != tile_ref.collision_border_bits)))
		{
			layer.tilelayer.set_collision(true, tile_ref.collision_border_bits);
			LOG_INFO("enable collision");
		}
		if ((!layer.tilelayer.has_scroll() && tile_ref.has_scroll) ||
			(tile_ref.has_scroll && (layer.tilelayer.get_scrollrate() != tile_ref.scrollrate)))
		{
			layer.tilelayer.set_scroll(true, tile_ref.scrollrate);
			LOG_INFO("enable scroll");
		}
		if ((!layer.tilelayer.has_parallax() && tile_ref.has_parallax) ||
			(tile_ref.has_parallax && (layer.tilelayer.get_parallax_size() != tile_ref.parallaxSize)))
		{
			layer.tilelayer.set_parallax(true, tile_ref.parallaxSize);
			LOG_INFO("enable parallax");
		}

		// update tiles
		for (unsigned yy = 0; yy < height; yy++) {
			for (unsigned xx = 0; xx < width; xx++) {

				Vec2u pos{ xx, yy };
				//size_t ndx = (size_t)xx + yy * width;

				if (tile_ndx >= tile_ref.tiles.size())
					break;

				auto& tilelayer = layer.tilelayer;

				if (auto& tile = tile_ref.tiles.at(tile_ndx); tile.tilePos == pos) {

					if ( (!tilelayer.hasTileAt(pos))
						|| (tilelayer.getTileTexPos(pos).value() != tile.texPos)
						|| (tilelayer.getTileTileset(pos)->getAssetName() != tile.tilesetName)) 
					{
						paint_count++;
						select_tileset(tile.tilesetName);
						select_tile(tile.texPos);
						paint_tile(pos);
					}
					tile_ndx++;
				}
				else if (tilelayer.hasTileAt(pos)) {
					erase_count++;
					erase_tile(pos);
				}
			}
		}


		// predraw to apply changes
		layer.tilelayer.predraw(0.0);

		it++; // increment to next tilelayer ref

		LOG_INFO("updated layer #{}: {} tile affected", layer.tilelayer.getID(), paint_count + erase_count);
	}


	// step 4: check objects



	std::chrono::duration<double> duration = std::chrono::system_clock::now() - start;
	LOG_INFO("apply duration: {}s", duration.count());


	return true;
}

}
