#include "fastfall/game/level/LevelEditor.hpp"

#include "fastfall/resource/Resources.hpp"

namespace ff {


LevelEditor::LevelEditor(Level* lvl) 
{
	level = lvl;
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

bool LevelEditor::select_layer(unsigned id)
{
	auto& bg = level->getBGLayers();
	auto& fg = level->getFGLayers();

	bool bg_found = false;
	auto it = std::find_if(bg.begin(), bg.end(),
		[id](const TileLayer& layer) {
			return layer.getID() == id;
		});

	if (it == bg.end()) {
		it = std::find_if(fg.begin(), fg.end(),
			[id](const TileLayer& layer) {
				return layer.getID() == id;
			});
	}
	else {
		bg_found = true;
	}

	if (bg_found || it != fg.end()) {
		layer_id = it->getID();
		is_layer_bg = bg_found;
		return true;
	}
	layer_id = 0;
	is_layer_bg = false;
	return false;
}

bool LevelEditor::insert_layer(unsigned before_id)
{
	return false;
}

bool LevelEditor::paint_tile(Vec2u pos)
{
	if (auto* layer = get_layer(layer_id, is_layer_bg); layer && curr_tileset) {
		layer->setTile(pos, tileset_pos, *curr_tileset);
	}
	return false;
}
bool LevelEditor::erase_tile(Vec2u pos)
{
	if (auto* layer = get_layer(layer_id, is_layer_bg)) {
		layer->removeTile(pos);
	}
	return false;
}

bool LevelEditor::select_tileset(std::string_view tileset_name)
{
	curr_tileset = Resources::get<TilesetAsset>(tileset_name);
	tileset_pos = Vec2u{};
	return curr_tileset != nullptr;
}
bool LevelEditor::select_tile(Vec2u tile_pos)
{
	if (curr_tileset 
		&& tile_pos.x < curr_tileset->getTileSize().x
		&& tile_pos.y < curr_tileset->getTileSize().y)
	{
		tileset_pos = tile_pos;
	}
	return false;
}

bool LevelEditor::set_name(std::string name)
{
	level->set_name(name);
	return true;
}
bool LevelEditor::set_bg_color(Color bg_color)
{
	level->set_bg_color(bg_color);
	return true;
}

bool LevelEditor::set_boundary(bool north, bool east, bool south, bool west)
{
	unsigned bits = (north ? cardinalToBits(Cardinal::NORTH) : 0u)
		| (east ? cardinalToBits(Cardinal::EAST) : 0u)
		| (south ? cardinalToBits(Cardinal::SOUTH) : 0u)
		| (west ? cardinalToBits(Cardinal::WEST) : 0u);

	level->set_borders(0u);
	level->set_borders(bits);
	return true;
}

bool LevelEditor::create_object(ObjectRef ref)
{
	return false;
}

unsigned LevelEditor::get_selected_layer()
{
	return layer_id;
}

const TilesetAsset* LevelEditor::get_selected_tileset()
{
	return curr_tileset;
}

Vec2u LevelEditor::get_selected_tile()
{
	return tileset_pos;
}

TileLayer* LevelEditor::get_layer(unsigned id, bool is_bg)
{
	if (level && id > 0)
	{
		if (is_bg) 
		{
			auto& bg = level->getBGLayers();
			auto it = std::find_if(bg.begin(), bg.end(),
				[id](const TileLayer& layer) {
					return layer.getID() == id;
				});

			if (it != bg.end()) {
				return &*it;
			}
		}
		else 
		{
			auto& fg = level->getFGLayers();
			auto it = std::find_if(fg.begin(), fg.end(),
				[id](const TileLayer& layer) {
					return layer.getID() == id;
				});

			if (it != fg.end()) {
				return &*it;
			}
		}
	}
	return nullptr;
}

// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;


bool LevelEditor::applyCommand(const EditCommand& cmd)
{
	return std::visit(
		overloaded{
			[this](SelectLayerCmd	cmd) { return select_layer(cmd.layer_id); },
			[this](InsertLayerCmd	cmd) { return insert_layer(cmd.layer_id); },
			[this](PaintTileCmd		cmd) { return paint_tile(cmd.pos); },
			[this](EraseTileCmd		cmd) { return erase_tile(cmd.pos); },
			[this](SelectTilesetCmd	cmd) { return select_tileset(cmd.name); },
			[this](SelectTileCmd	cmd) { return select_tile(cmd.tileset_pos); },
			[this](SetNameCmd		cmd) { return set_name(std::string(cmd.name)); },
			[this](SetBGColorCmd	cmd) { return set_bg_color(cmd.color); },
			[this](SetBoundary		cmd) { return set_boundary(cmd.north, cmd.east, cmd.south, cmd.west); },
			[this](CreateObjCmd		cmd) { return create_object(cmd.ref); },
		},
		cmd
	);
}





















}