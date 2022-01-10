#pragma once

#include "fastfall/game/level/Tile.hpp"
#include "fastfall/resource/asset/LevelAssetTypes.hpp"
#include "fastfall/util/xml.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/util/log.hpp"
#include "fastfall/util/grid_vector.hpp"

#include <vector>
#include <map>
#include <string_view>
#include <optional>

namespace ff {


// represents a layer of tile data
class TileLayerData {
private:
	Vec2u tileSize;

	bool has_parallax = false;
	bool has_scroll = false;
	bool has_collision = false;

	Vec2u parallaxSize;
	Vec2f scrollrate;
	unsigned collision_border_bits = 0u;

	struct TileData {
		bool has_tile		= false;
		bool is_autotile	= false;
		Vec2u	pos			= {};
		TileID	tile_id		= {};
		uint8_t tileset_ndx = UINT8_MAX;
	};
	grid_vector<TileData> tiles;
	grid_vector<TileShape> shapes;

	struct TilesetData {
		std::string name;
		unsigned tile_count;
	};
	std::vector<TilesetData> tilesets;

	unsigned layer_id = 0;

public:

	TileLayerData();
	TileLayerData(unsigned id);
	TileLayerData(unsigned id, Vec2u size);

	inline bool hasParallax() const { return has_parallax; };
	inline Vec2u getParallaxSize() const { return parallaxSize; };

	inline bool hasScrolling() const { return has_scroll; };
	inline Vec2f getScrollRate() const { return scrollrate; };

	inline bool hasCollision() const { return has_collision; };
	inline unsigned getCollisionBorders() const { return collision_border_bits; }

	inline Vec2u getSize() const { return tileSize; };

	inline unsigned getID() const { return layer_id; };

	[[nodiscard]]
	static TileLayerData loadFromTMX(rapidxml::xml_node<>* layerNode, const TilesetMap& tilesets);

	void resize(Vec2u size, Vec2i offset = Vec2i{ 0, 0 });

	void setParallax(bool enabled, Vec2u parallax_size = Vec2u{});

	void setScroll(bool enabled, Vec2f scroll_rate = Vec2f{});

	void setCollision(bool enabled, unsigned border = 0);

	// returns tile id actually set after autotile
	TileID setTile(Vec2u at, TileID tile_id, const TilesetAsset& tileset);

	std::pair<bool, unsigned> removeTile(Vec2u at);

	void clearTiles();

	const grid_vector<TileData>& getTileData() const { return tiles; };

	inline const auto& getTilesets() const { return tilesets; };

	const std::string* getTilesetFromNdx(uint8_t ndx) const {
		return ndx < tilesets.size() ? &tilesets.at(ndx).name : nullptr;
	}
};

}