#pragma once

#include "fastfall/game/Level.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"

#include <string_view>
#include <variant>

namespace ff {

class LevelEditor {
public:
	constexpr static Vec2u MIN_LEVEL_SIZE = Vec2u{ GAME_TILE_W, GAME_TILE_H };

	// attach to existing level
	LevelEditor(Level* lvl);

	// create a new level
	LevelEditor(GameContext context, std::string name = "New Level", Vec2u tile_size = MIN_LEVEL_SIZE); 

	bool select_layer(unsigned id);
	bool insert_layer(unsigned before_id);

	bool paint_tile(Vec2u pos);
	bool erase_tile(Vec2u pos);

	bool select_tileset(std::string_view tileset_name);
	bool select_tile(Vec2u tile_pos);

	bool set_name(std::string name);
	bool set_bg_color(Color bg_color);

	bool set_boundary(bool north, bool east, bool south, bool west);

	bool create_object(ObjectRef ref);

	unsigned get_selected_layer();
	const TilesetAsset* get_selected_tileset();
	Vec2u get_selected_tile();


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