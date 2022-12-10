#pragma once

#include "fastfall/game/tile/Tile.hpp"
#include "fastfall/resource/asset/LevelAssetTypes.hpp"
#include "fastfall/util/xml.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/util/log.hpp"
#include "fastfall/util/grid_vector.hpp"

#include <vector>
#include <map>
#include <string_view>
#include <optional>
#include <array>

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
		TileID	base_id		= {};
		TileID	tile_id		= {};
		uint8_t tileset_ndx = UINT8_MAX;
	};
	grid_vector<TileData> tiles;
	grid_vector<TileShape> shapes;

	struct TilesetData {
		const TilesetAsset* tileset;
		unsigned tile_count;
	};
	std::vector<TilesetData> tilesets;

	unsigned layer_id = 0;
	std::string layer_name;
    TileShape autotile_substitute = "solid"_ts;

public:

	struct TileChange {
		//uint8_t last_tileset_ndx = UINT8_MAX;
		const TilesetAsset* tileset;
		Vec2u position;
	};

	struct TileChangeArray
	{
		void push(TileChange change)
		{
			arr[count++] = change;
		}

		uint8_t count = 0;
		std::array<TileChange, 9> arr;
	};

	struct RemoveResult {
		bool erased_tile = false;
		unsigned tileset_remaining;
		TileChangeArray changes;
	};

	//using TileChangeArray = std::array<TileChange, 9>;

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

	TileChangeArray setTile(Vec2u at, TileID tile_id, const TilesetAsset& tileset);
	RemoveResult removeTile(Vec2u at);

	std::string_view getName() const { return layer_name; };
	void setName(std::string_view name) { layer_name = name; };

	void clearTiles();

	const grid_vector<TileData>& getTileData() const { return tiles; };
	const grid_vector<TileShape>& getTileShapes() const { return shapes; };

	inline const auto& getTilesets() const { return tilesets; };

	const TilesetAsset* getTilesetFromNdx(uint8_t ndx) const {
		return ndx < tilesets.size() ? tilesets.at(ndx).tileset : nullptr;
	}

    void set_autotile_substitute(TileShape sub) noexcept { autotile_substitute = sub; }
    TileShape get_autotile_substitute() const noexcept { return autotile_substitute; }

private:

	void setShape(Vec2u at, TileShape shape, TileChangeArray& changes);

};

}