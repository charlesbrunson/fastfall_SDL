#pragma once

#include "fastfall/game/Level.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"

#include <string_view>
#include <variant>

#include <iterator>

namespace ff {


struct LayerPosition {
	enum class Type {
		At,
		Start,
		End
	} type;
	int position;

	void update(const Level* lvl) {
		int layer_count = lvl->get_layers().size();
		if (layer_count == 0) {
			position = 0;
			return;
		}

		int fgNdx = lvl->getFGStartNdx();

		if (position >= (layer_count - fgNdx))
		{
			type = LayerPosition::Type::End;
		}
		else if (position < (-fgNdx))
		{
			type = LayerPosition::Type::Start;
		}
	}

	static LayerPosition Start() {
		return LayerPosition{
			.type = Type::Start,
			.position = 0
		};
	};
	static LayerPosition End() {
		return LayerPosition{
			.type = Type::End,
			.position = 0
		};
	};
	static LayerPosition At(int layer_pos) {
		return LayerPosition{
			.type = Type::At,
			.position = layer_pos
		};
	};
	static LayerPosition Foreground() {
		return LayerPosition{
			.type = Type::At,
			.position = 0
		};
	};
};


class LevelEditor {
public:
	constexpr static Vec2u MIN_LEVEL_SIZE = Vec2u{ GAME_TILE_W, GAME_TILE_H };

	// CONSTRUCTORS

	// attach to existing level
	LevelEditor(Level& lvl, bool show_imgui);

	// create a new level
	LevelEditor(GameContext context, bool show_imgui, std::string name = "New Level", Vec2u tile_size = MIN_LEVEL_SIZE);

	~LevelEditor();

	bool is_attached() {
		return level && level->is_attached(this);
	}

	bool reattach() {
		if (level) {
			level->attach(this);

			// references may be stale
			obj_layer_selected = false;
			curr_layer = nullptr;
		}
		return level;
	}

	// LAYERS

	// create layer at position, selects it
	bool create_layer(LayerPosition layer_pos); 

	// select layer at positon (start and end specify the first and last layer, respectively)
	bool select_layer(LayerPosition layer_pos);
	void select_obj_layer();
	void deselect_layer();

	// move selected layer to new position
	// retains selection of moved layer
	bool move_layer(LayerPosition layer_pos);

	// erases selected layer
	// deselects layer
	bool erase_layer();

	// LAYER PROPERTIES

	bool layer_set_collision(bool enabled, unsigned borderBits = 0u);
	bool layer_set_scroll(bool enabled, Vec2f scroll_rate = Vec2f{});
	bool layer_set_parallax(bool enabled, Vec2u parallax_size = Vec2u{});

	// TILES

	// paints tile onto selected layer, using selected tileset and tile
	bool paint_tile(Vec2u pos);

	// paints tile onto selected layer
	bool erase_tile(Vec2u pos);

	// TILESET

	// selects tileset for painting tiles
	bool select_tileset(std::string_view tileset_name);
	void deselect_tileset();

	// selects tile from selected tileset for painting tiles
	bool select_tile(Vec2u tile_pos);
	void deselect_tile();

	// LEVEL PROPERTIES

	// changes level's name
	bool set_name(std::string name);

	// changes level's background color
	bool set_bg_color(Color bg_color);

	// resizes level
	bool set_size(Vec2u size);

	// OBJECTS - TODO LATER

	/*
	
	// selects existing object ref
	bool select_object(object_id id);

	// adds an object ref to the level, selects it
	bool create_object(ObjectRef ref);

	// updates selected object with the passed ref data
	bool update_object(ObjectRef ref);

	// removes selected object ref
	bool remove_object();

	*/

	std::optional<Vec2u> get_tile() const { return tileset_pos; };
	const TilesetAsset* get_tileset() const { return curr_tileset; };

	bool applyLevelAsset(const LevelAsset* asset);

protected:

	std::unique_ptr<Level> created_level = nullptr;
	Level* level = nullptr;								// pointer to level being edited, may point externally or to created_level

	bool obj_layer_selected = false;
	LevelTileLayer* curr_layer = nullptr;				// current tile layer

	const TilesetAsset* curr_tileset = nullptr;			// current tileset
	std::optional<Vec2u> tileset_pos = std::nullopt;	// current tile from tileset
};






}
