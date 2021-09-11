#pragma once

#include "fastfall/game/Level.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"

#include <string_view>

namespace ff {

class LevelEditor {
public:

	constexpr static Vec2u MIN_LEVEL_SIZE = Vec2u{ GAME_TILE_W, GAME_TILE_H };

	[[nodiscard]]
	static LevelEditor attach(Level* lvl);

	[[nodiscard]]
	static LevelEditor create_level(std::string name = "New Level", Vec2u tile_size = MIN_LEVEL_SIZE);

	static void sync_with_asset(Level* lvl, LevelAsset* asset);

	void select_layer(unsigned id);
	void insert_layer(unsigned before_id);

	void paint_tile(Vec2u pos);
	void erase_tile(Vec2u pos);

	void select_tileset(std::string_view tileset_name);
	void select_tileset(const TilesetAsset* tileset);
	void select_tile(Vec2u tileset_pos);

	void set_name(std::string name);
	void set_bg_color(Color bg_color);

	void set_boundary(bool north, bool east, bool south, bool west);

	void create_object(ObjectRef ref);

protected:
	constexpr LevelEditor() {};

	std::unique_ptr<Level> created_level = nullptr;
	Level* level;

	unsigned layer_id = 0;
	unsigned layer_counter = 0;

	enum class LayerType {
		None,
		Object,
		Tile
	} curr_layer_type = LayerType::None;

	const TilesetAsset* curr_tileset = nullptr;
	Vec2u tileset_pos;

};

}