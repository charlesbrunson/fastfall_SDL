#include "fastfall/game/level/LevelEditor.hpp"

#include "fastfall/resource/Resources.hpp"

namespace ff {


LevelEditor::LevelEditor(Level& lvl, bool show_imgui) 
	: display_imgui(show_imgui)
{
	level = &lvl;
	assert(level);
	if (!level->hasEditorHooked) {
		level->hasEditorHooked = true;
	}
	else {
		LOG_WARN("editor failed to hook level, already hooked");
		level = nullptr;
	}
}

LevelEditor::LevelEditor(GameContext context, bool show_imgui, std::string name, Vec2u tile_size)
	: display_imgui(show_imgui)
{
	assert(tile_size.x >= LevelEditor::MIN_LEVEL_SIZE.x);
	assert(tile_size.y >= LevelEditor::MIN_LEVEL_SIZE.y);

	created_level = std::make_unique<Level>(context);
	level = created_level.get();

	assert(level);
}

LevelEditor::~LevelEditor() {
	if (level->hasEditorHooked) {
		level->hasEditorHooked = false;
	}
	else {
		LOG_WARN("editor failed to unhook level, already unhooked");
	}
}

// LAYERS

// create layer at position, selects it
bool LevelEditor::create_layer(LayerPosition layer_pos)
{
	if (!level) return false;

	int layer_count = level->getTileLayers().size();
	int fgNdx = level->getFGStartNdx();

	if (layer_pos.position >= (layer_count - fgNdx))
	{
		layer_pos.type = LayerPosition::Type::End;
	}
	else if (layer_pos.position < (-fgNdx))
	{
		layer_pos.type = LayerPosition::Type::Start;
	}

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

	level->insertTileLayer(LevelLayer{
		.position = layer_pos.position,
		.tilelayer = TileLayer{level->getContext(), 0, level->size() } // todo get an actual id
		});

	return false;
}

// select layer at positon (start and end specify the first and last layer, respectively)
bool LevelEditor::select_layer(LayerPosition layer_pos)
{
	if (!level) return false;

	int layer_count = level->getTileLayers().size();
	if (layer_count == 0)
		return false;

	int fgNdx = level->getFGStartNdx();

	if (layer_pos.position >= (layer_count - fgNdx))
	{
		layer_pos.type = LayerPosition::Type::End;
	}
	else if (layer_pos.position < (-fgNdx)) 
	{
		layer_pos.type = LayerPosition::Type::Start;
	}

	switch (layer_pos.type)
	{
	case LayerPosition::Type::Start:
		curr_layer = &level->getTileLayers().at(0);
		break;
	case LayerPosition::Type::End:
		curr_layer = &level->getTileLayers().back();
		break;
	case LayerPosition::Type::At:
		curr_layer = &level->getTileLayers().at((size_t)(layer_pos.position + fgNdx));
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

	if (!level) return false;


	if (obj_layer_selected)
	{
		// ...
	}
	else if (curr_layer)
	{
		// ...
	}
	return false;
}

// erases selected layer
// deselects layer
bool LevelEditor::erase_layer()
{
	if (!level) return false;

	if (curr_layer)
	{

	}
	return false;
}

// TILES

// paints tile onto selected layer, using selected tileset and tile
bool LevelEditor::paint_tile(Vec2u pos)
{
	if (!level) return false;

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
	if (!level) return false;

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
	if (!level) return false;

	curr_tileset = Resources::get<TilesetAsset>(tileset_name);
	deselect_tile();
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

// changes level's boundary collision
bool LevelEditor::set_boundary(bool north, bool east, bool south, bool west)
{
	if (!level) return false;

	unsigned bits = 
		  (north ? cardinalToBits(Cardinal::NORTH) : 0u)
		| (east  ? cardinalToBits(Cardinal::EAST)  : 0u)
		| (south ? cardinalToBits(Cardinal::SOUTH) : 0u)
		| (west  ? cardinalToBits(Cardinal::WEST)  : 0u);

	level->set_borders(0u);
	level->set_borders(bits);
	return true;
}
bool LevelEditor::set_boundary(unsigned cardinalBits)
{
	if (!level) return false;

	level->set_borders(0u);
	level->set_borders(cardinalBits);
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

	// step 1: level size and other properties
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

	// step 2: correct layers and ordering

	// get the layer ids & order from the asset
	std::vector<unsigned> asset_ids;
	std::transform(asset->getLayerRefs()->begin(), asset->getLayerRefs()->end(),
			std::back_inserter(asset_ids), 
			[](const LayerRef& layer) -> unsigned {
				return layer.type == LayerRef::Type::Tile ? layer.id : 0; // mark obj layer as zero
			});

	unsigned asset_fg_ndx = std::distance(asset_ids.cbegin(), std::find(asset_ids.cbegin(), asset_ids.cend(), 0));

	unsigned level_ndx = 0;
	auto& layers = level->getTileLayers();

	std::set<unsigned> nLayers;
	for (unsigned asset_ndx = 0; asset_ndx < asset_ids.size(); asset_ndx++)
	{
		unsigned asset_layer_id = asset_ids[asset_ndx];

		if (layers[level_ndx].tilelayer.getID() != asset_layer_id)
		{
			// try to find the correct id in the level
			auto it = std::find_if(layers.begin() + level_ndx, layers.end(),
					[asset_layer_id](const LevelLayer& layer) {
						return layer.tilelayer.getID() == asset_layer_id;
					});

			int layer_pos = (int)asset_ndx - asset_fg_ndx;
			bool is_bg = layer_pos < 0;

			if (it != layers.end()) {
				select_layer(LayerPosition::At(it->position));
				move_layer(LayerPosition::At(layer_pos));
			}
			else {
				create_layer(LayerPosition::At(layer_pos));		
				curr_layer->tilelayer = TileLayer{
					level->getContext(),
					asset_layer_id,
					asset->getLayerRefs()->at(asset_ndx).asTileLayer(),	
					layer_pos == 0
				};
			}
		}
	}


	// step 3: per layer check each tile

	// step 4: check objects

	// step 5: apply boundary
	set_boundary(asset->getBorder());

	return true;
}

}
