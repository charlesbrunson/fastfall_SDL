#pragma once

#include "fastfall/resource/asset/LevelAssetTypes.hpp"
#include "fastfall/util/xml.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/util/log.hpp"

#include <vector>
#include <map>

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

	// tile data
	struct TileData {
		std::vector<bool> has_tile;
		std::vector<Vec2u> pos;
		std::vector<Vec2u> tex_pos;
		std::vector<uint8_t> tileset_ndx;
	} tiles;

	std::vector<std::pair<std::string, unsigned>> tilesets;

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

	void setTile(Vec2u at, Vec2u tex, const std::string& tileset);

	std::pair<bool, unsigned> removeTile(Vec2u at);

	void clearTiles();

	const TileData& getTileData() const { return tiles; };

	inline const auto& getTilesets() const { return tilesets; };

	const std::string* getTilesetFromNdx(uint8_t ndx) const {
		return ndx < tilesets.size() ? &tilesets.at(ndx).first : nullptr;
	}
};

}