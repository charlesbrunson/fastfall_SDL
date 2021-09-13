#include "fastfall/game/level/LevelEditor.hpp"

#include "fastfall/resource/Resources.hpp"

namespace ff {


LevelEditor::LevelEditor(Level& lvl) 
{
	level = &lvl;
	assert(level);
}

LevelEditor::LevelEditor(GameContext context, std::string name, Vec2u tile_size)
{
	assert(tile_size.x >= LevelEditor::MIN_LEVEL_SIZE.x);
	assert(tile_size.y >= LevelEditor::MIN_LEVEL_SIZE.y);

	created_level = std::make_unique<Level>(context);
	level = created_level.get();

	assert(level);
}

// LAYERS

// create layer at position, selects it
bool LevelEditor::create_layer(LayerPosition layer_pos)
{
	return false;
}

// select layer at positon (start and end specify the first and last layer, respectively)
bool LevelEditor::select_layer(LayerPosition layer_pos)
{
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
		using enum LayerPosition::Type;
	case Start:
		curr_layer = &level->getTileLayers().at(0);
		break;
	case End:
		curr_layer = &level->getTileLayers().back();
		break;
	case At:
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
	if (obj_layer_selected)
	{
		// ...
	}
	else if (curr_layer)
	{

	}
	return false;
}


// TILES

// paints tile onto selected layer, using selected tileset and tile
bool LevelEditor::paint_tile(Vec2u pos)
{
	if (curr_layer && curr_tileset && tileset_pos) {
		curr_layer->tilelayer.setTile(pos, *tileset_pos, *curr_tileset);
		return true;
	}
	return false;
}

// paints tile onto selected layer
bool LevelEditor::erase_tile(Vec2u pos)
{
	if (curr_layer) {
		curr_layer->tilelayer.removeTile(pos);
		return true;
	}
	return false;
}

// TILESET

// selects tileset for painting tiles
bool LevelEditor::select_tileset(std::string_view tileset_name)
{
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
	level->set_name(name);
	return true;
}

// changes level's background color
bool LevelEditor::set_bg_color(Color bg_color)
{
	level->set_bg_color(bg_color);
	return true;
}

// changes level's boundary collision
bool LevelEditor::set_boundary(bool north, bool east, bool south, bool west)
{
	unsigned bits = 
		  (north ? cardinalToBits(Cardinal::NORTH) : 0u)
		| (east  ? cardinalToBits(Cardinal::EAST)  : 0u)
		| (south ? cardinalToBits(Cardinal::SOUTH) : 0u)
		| (west  ? cardinalToBits(Cardinal::WEST)  : 0u);

	level->set_borders(0u);
	level->set_borders(bits);
	return true;
}



// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;


bool LevelEditor::applyCommand(const EditCommand& cmd)
{
	return std::visit(
		overloaded{
			[this](SelectLayerCmd	cmd) { return select_layer(cmd.layerpos); },
			[this](CreateLayerCmd	cmd) { return create_layer(cmd.layerpos); },
			[this](MoveLayerCmd		cmd) { return create_layer(cmd.layerpos); },

			[this](PaintTileCmd		cmd) { return paint_tile(cmd.pos); },
			[this](EraseTileCmd		cmd) { return erase_tile(cmd.pos); },

			[this](SelectTilesetCmd	cmd) { return select_tileset(cmd.name); },
			[this](SelectTileCmd	cmd) { return select_tile(cmd.tileset_pos); },

			[this](SetNameCmd		cmd) { return set_name(std::string(cmd.name)); },
			[this](SetBGColorCmd	cmd) { return set_bg_color(cmd.color); },
			[this](SetBoundary		cmd) { return set_boundary(cmd.north, cmd.east, cmd.south, cmd.west); }
		},
		cmd
	);
}





















}