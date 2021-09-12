#pragma once

#include "fastfall/game/Level.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"

#include <string_view>
#include <variant>

namespace ff {

struct LayerPosition {
	enum class Position {
		AtLayer,
		Start,
		End
	} type;
	unsigned at_layer_id;

	static LayerPosition Begin() {
		return LayerPosition{
			.type = Position::Start,
			.at_layer_id = 0u
		};
	};
	static LayerPosition End() {
		return LayerPosition{
			.type = Position::End,
			.at_layer_id = 0u
		};
	};
	static LayerPosition At(unsigned layer_id) {
		return LayerPosition{
			.type = Position::AtLayer,
			.at_layer_id = layer_id
		};
	};
};

class LevelEditor {
public:
	constexpr static Vec2u MIN_LEVEL_SIZE = Vec2u{ GAME_TILE_W, GAME_TILE_H };

	// CONSTRUCTORS

	// attach to existing level
	LevelEditor(Level* lvl);

	// create a new level
	LevelEditor(GameContext context, std::string name = "New Level", Vec2u tile_size = MIN_LEVEL_SIZE); 

	// LAYERS

	// create layer at position, selects it
	bool create_layer(LayerPosition layer_pos); 

	// select layer at positon (start and end specify the first and last layer, respectively)
	bool select_layer(LayerPosition layer_pos); 

	// move selected layer to new position
	// retains selection of moved layer
	bool move_layer(LayerPosition layer_pos);   

	// TILES

	// paints tile onto selected layer, using selected tileset and tile
	bool paint_tile(Vec2u pos);

	// paints tile onto selected layer
	bool erase_tile(Vec2u pos);

	// TILESET

	// selects tileset for painting tiles
	bool select_tileset(std::string_view tileset_name);

	// selects tile from selected tileset for painting tiles
	bool select_tile(Vec2u tile_pos);

	// LEVEL PROPERTIES

	// changes level's name
	bool set_name(std::string name);

	// changes level's background color
	bool set_bg_color(Color bg_color);

	// changes level's boundary collision
	bool set_boundary(bool north, bool east, bool south, bool west);

	// OBJECTS

	// selects existing object ref
	bool select_object(object_id id);

	// adds an object ref to the level, selects it
	bool create_object(ObjectRef ref);

	// updates selected object with the passed ref data
	bool update_object(ObjectRef ref);

	// removes selected object ref
	bool remove_object();

	// ADDT'L

	// selection state
	unsigned get_selected_layer_id();
	LayerPosition get_selected_layer_pos();
	const TileLayer& get_selected_layer();

	const TilesetAsset* get_selected_tileset();

	Vec2u get_selected_tile();

	const ObjectRef& get_selected_object();

	// other
	const TileLayer& get_layer(LayerPosition layer_pos);

protected:

	struct SelectLayerCmd	{ unsigned layer_id; };
	struct InsertLayerCmd	{ unsigned layer_id; };
	struct PaintTileCmd		{ Vec2u pos; };
	struct EraseTileCmd		{ Vec2u pos; };
	struct SelectTilesetCmd { std::string_view name; };
	struct SelectTileCmd	{ Vec2u tileset_pos; };
	struct SetNameCmd		{ std::string_view name; };
	struct SetBGColorCmd	{ Color color; };
	struct SetBoundary		{ bool north; bool east; bool south; bool west; };
	struct CreateObjCmd		{ ObjectRef ref; };

	using EditCommand = std::variant<
		SelectLayerCmd,
		InsertLayerCmd,
		PaintTileCmd,
		EraseTileCmd,
		SelectTilesetCmd,
		SelectTileCmd,
		SetNameCmd,
		SetBGColorCmd,
		SetBoundary,
		CreateObjCmd
	>;

	bool applyCommand(const EditCommand& cmd);

	TileLayer* get_layer(unsigned id, bool is_bg);

	std::unique_ptr<Level> created_level = nullptr;
	Level* level = nullptr;

	unsigned layer_id = 0;
	unsigned layer_counter = 0;
	bool is_layer_bg = false;

	enum class LayerType {
		None,
		Object,
		Tile
	} curr_layer_type = LayerType::None;

	const TilesetAsset* curr_tileset = nullptr;
	Vec2u tileset_pos;

};






}