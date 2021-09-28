#include "fastfall/game/level/LevelEditor.hpp"

#include "fastfall/resource/Resources.hpp"

#include <chrono>

namespace ff {


LevelEditor::LevelEditor(Level& lvl, bool show_imgui) 
{
	level = &lvl;
	assert(level);

	level->attach(this);
}

LevelEditor::LevelEditor(GameContext context, bool show_imgui, std::string name, Vec2u tile_size)
{
	assert(tile_size.x >= LevelEditor::MIN_LEVEL_SIZE.x);
	assert(tile_size.y >= LevelEditor::MIN_LEVEL_SIZE.y);

	created_level = std::make_unique<Level>(context);
	level = created_level.get();

	assert(level);
}

LevelEditor::~LevelEditor() {
	if (level) {
		level->detach(this);
	}
}

// LAYERS

// create layer at position, selects it
bool LevelEditor::create_layer(LayerPosition layer_pos)
{
	if (!is_attached()) return false;

	layer_pos.update(level);

	int layer_count = level->getTileLayers().size();
	int fgNdx = level->getFGStartNdx();

	switch (layer_pos.type)
	{
	case LayerPosition::Type::Start:
		layer_pos.position = -fgNdx - 1;
		break;
	case LayerPosition::Type::End:
		layer_pos.position = layer_count - fgNdx + 1;
		break;
	case LayerPosition::Type::At:
		break;
	}

	curr_layer = &level->insertTileLayer(LevelLayer{
		.position = layer_pos.position,
		.tilelayer = TileLayer{level->getContext(), 0, level->size() } // todo get an actual id
		});

	return true;
}

// select layer at positon (start and end specify the first and last layer, respectively)
bool LevelEditor::select_layer(LayerPosition layer_pos)
{
	if (!is_attached()) return false;

	layer_pos.update(level);

	switch (layer_pos.type)
	{
	case LayerPosition::Type::Start:
		curr_layer = &level->getTileLayers().at(0);
		break;
	case LayerPosition::Type::End:
		curr_layer = &level->getTileLayers().back();
		break;
	case LayerPosition::Type::At:
		curr_layer = &level->getTileLayers().at((size_t)(layer_pos.position + level->getFGStartNdx()));
		break;
	}

	if (!curr_layer) 
	{
		deselect_layer();
	}
	return curr_layer;
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
bool LevelEditor::move_layer(LayerPosition layer_pos)
{

	if (!is_attached()) return false;

	layer_pos.update(level);
	int layer_count = level->getTileLayers().size();
	int fgNdx = level->getFGStartNdx();

	if (obj_layer_selected)
	{
		// ...
	}
	else if (curr_layer)
	{
		TileLayer layer = std::move(curr_layer->tilelayer);
		level->removeTileLayer(curr_layer->position);
		level->insertTileLayer(
			LevelLayer{
				.position = layer_pos.position,
				.tilelayer = std::move(layer)
			}
		);
		return true;
	}
	return false;
}

// erases selected layer
// deselects layer
bool LevelEditor::erase_layer()
{
	if (level && curr_layer)
	{
		level->removeTileLayer(curr_layer->position);	
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
	if (!is_attached()) return false;

	if (curr_layer && curr_tileset && tileset_pos) {
		Vec2u size = curr_layer->tilelayer.getSize();

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
	if (!is_attached()) return false;

	if (curr_layer) {
		Vec2u size = curr_layer->tilelayer.getSize();

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
	if (!is_attached()) return false;

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
	if (!is_attached()) return false;

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
	if (!is_attached()) return false;

	level->set_name(name);
	return true;
}

// changes level's background color
bool LevelEditor::set_bg_color(Color bg_color)
{
	if (!is_attached()) return false;

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
	if (!is_attached()) return false;

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
	std::vector<unsigned> asset_ids;
	std::transform(asset->getLayerRefs()->begin(), asset->getLayerRefs()->end(),
		std::back_inserter(asset_ids),
		[](const LayerRef& layer) -> unsigned {
			return layer.type == LayerRef::Type::Tile ? layer.id : 0; // mark obj layer as zero
		});

	unsigned asset_fg_ndx = std::distance(asset_ids.cbegin(), std::find(asset_ids.cbegin(), asset_ids.cend(), 0));

	auto& layers = level->getTileLayers();
	std::set<unsigned> nLayers;

	auto create = [this, &nLayers, asset](LayerPosition pos, unsigned ndx, unsigned id) -> bool {
		bool ret;
		ret = create_layer(pos);

		assert(ret);

		curr_layer->tilelayer = TileLayer{
			level->getContext(),
			id,
			asset->getLayerRefs()->at(ndx).asTileLayer()
		};
		nLayers.insert(id);

		return ret;
	};


	{
		std::stringstream ss;
		unsigned i = 0;
		for (auto& layer : layers) {
			ss << layer.tilelayer.getID() << " ";
			i++;
		}
		LOG_INFO("level layer order: {}", ss.str());
	}
	{
		std::stringstream ss;
		for (unsigned id : asset_ids) {
			ss << id << " ";
		}
		LOG_INFO("asset layer order: {}", ss.str());
	}

	for (unsigned asset_ndx = 0; asset_ndx < asset_ids.size(); )
	{
		unsigned asset_layer_id = asset_ids[asset_ndx];
		int asset_pos = (int)asset_ndx - asset_fg_ndx;

		std::stringstream ss;

		unsigned layer_ndx = asset_ndx - (asset_pos > 0 ? 1 : 0); // remove object layer

		if (asset_layer_id == 0) // object layer
		{
			LOG_INFO("---OBJECT LAYER---")
			asset_ndx++;
			continue;
		};


		LOG_INFO("asset ndx: {}, layer id: {}, exp pos: {}", asset_ndx, asset_layer_id, asset_pos);

		log::scope sc;

		if (layer_ndx >= layers.size())
		{
			LOG_INFO("creating layer {} at end", asset_layer_id);

			// create layer
			create(LayerPosition::End(), asset_ndx, asset_layer_id);
			asset_ndx++;
		}
		else
		{
			int position = layers.at(layer_ndx).position;
			auto& layer = layers.at(layer_ndx).tilelayer;

			if (layer.getID() != asset_layer_id || position != asset_pos)
			{

				auto asset_has_level_layer_it = std::find_if(asset_ids.begin(), asset_ids.end(), [&layer](unsigned id) {
						return layer.getID() == id;
					});

				auto level_has_asset_layer_it = std::find_if(layers.begin(), layers.end(), [asset_layer_id](LevelLayer& t_layer) {
						return t_layer.tilelayer.getID() == asset_layer_id;
					});

				bool asset_has_level_layer = asset_has_level_layer_it != asset_ids.end();
				bool level_has_asset_layer = level_has_asset_layer_it != layers.end();


				LOG_INFO("layer id mismatch, level layer = {}, asset:{}, level:{}", layer.getID(), asset_has_level_layer, level_has_asset_layer);


				if (asset_has_level_layer && level_has_asset_layer)
				{
					// out of order
					if (select_layer(LayerPosition::At(level_has_asset_layer_it->position)))
					{

						LOG_INFO(" -- moving layer {} (at {}) to {}",
							level_has_asset_layer_it->tilelayer.getID(),
							level_has_asset_layer_it->position,
							asset_pos
						);


						// TODO PICK THE RIGHT POSITION TO MOVE TO:
						move_layer(LayerPosition::At(asset_pos));
					}
					asset_ndx++;

				}
				else if (!asset_has_level_layer)
				{
					//erase layer
					if (select_layer(LayerPosition::At(position)))
					{
						LOG_INFO(" -- erasing layer {} at {}",
							curr_layer->tilelayer.getID(),
							position
						);
						erase_layer();
					}
				}
				else if (!level_has_asset_layer)
				{
					// create layer
					LOG_INFO(" -- creating layer {} at {}",
						asset_layer_id,
						asset_pos
					);
					create(LayerPosition::At(asset_pos), asset_ndx, asset_layer_id);
					asset_ndx++;
				}
			}
			else {
				LOG_INFO("layer id match, level layer = {}", layer.getID());
				asset_ndx++;
			}
		}

		
		{
			std::stringstream ss;
			unsigned i = 0;
			for (auto& layer : layers) {
				ss << layer.tilelayer.getID() << (i == layer_ndx ? "|" : " ");
				i++;
			}
			LOG_INFO("level layer order: {}", ss.str());
		}

	}

	// step 3: per layer update
	auto it = asset->getLayerRefs()->begin();
	for (auto& layer : layers) 
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

		select_layer(LayerPosition::At(layer.position));

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
