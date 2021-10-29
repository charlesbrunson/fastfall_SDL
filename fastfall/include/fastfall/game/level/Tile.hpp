#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/cardinal.hpp"


namespace ff {

class TilesetAsset;

static constexpr unsigned TILE_TYPE_COUNT = 13U;

class TileShape {
public:
	enum class Type : uint8_t {
		EMPTY,
		SOLID,
		HALF,
		HALFVERT,
		SLOPE,
		SHALLOW1,
		SHALLOW2,
		STEEP1,
		STEEP2,
		ONEWAY,
		ONEWAY_WALL,

		//not for use in tilesets
		LEVELBOUNDARY,
		LEVELBOUNDARY_WALL,

	};

	TileShape() noexcept;
	TileShape(const char* shapeStr) noexcept;

	constexpr TileShape(Type tileType, bool vertFlip, bool horiFlip) noexcept :
		type(tileType),
		vflipped(vertFlip),
		hflipped(horiFlip)
	{
		init();
	}

	~TileShape() = default;

	Type type;
	uint8_t shapeTouches; // contains Cardinal bits
	bool hflipped = false;
	bool vflipped = false;
private:

	using TilePrototype = std::pair<TileShape::Type, uint8_t>;
	constexpr static std::array<TilePrototype, TILE_TYPE_COUNT> tilePrototypes{
		TilePrototype{TileShape::Type::EMPTY, 0u},
		{Type::SOLID,				cardinalToBits(Cardinal::NORTH, Cardinal::EAST, Cardinal::SOUTH, Cardinal::WEST)},
		{Type::HALF,				cardinalToBits(Cardinal::EAST, Cardinal::SOUTH, Cardinal::WEST)},
		{Type::HALFVERT,			cardinalToBits(Cardinal::NORTH,                 Cardinal::SOUTH, Cardinal::WEST)},
		{Type::SLOPE,				cardinalToBits(Cardinal::EAST, Cardinal::SOUTH)},
		{Type::SHALLOW1,			cardinalToBits(Cardinal::EAST, Cardinal::SOUTH)},
		{Type::SHALLOW2,			cardinalToBits(Cardinal::EAST, Cardinal::SOUTH, Cardinal::WEST)},
		{Type::STEEP1,				cardinalToBits(Cardinal::NORTH, Cardinal::EAST, Cardinal::SOUTH)},
		{Type::STEEP2,				cardinalToBits(Cardinal::EAST, Cardinal::SOUTH)},
		{Type::ONEWAY,				cardinalToBits(Cardinal::NORTH)},
		{Type::ONEWAY_WALL,			cardinalToBits(Cardinal::EAST)},

		{Type::LEVELBOUNDARY,		cardinalToBits(Cardinal::NORTH)},
		{Type::LEVELBOUNDARY_WALL,	cardinalToBits(Cardinal::EAST)}
	};

	constexpr void init() {

		shapeTouches = std::find_if(tilePrototypes.begin(), tilePrototypes.end(), [this](const TilePrototype& element) {
			return element.first == type;
			})->second;

		// flip touch flags
		if (hflipped) {
			bool east = (shapeTouches & cardinalBit[Cardinal::EAST]) > 0;
			bool west = (shapeTouches & cardinalBit[Cardinal::WEST]) > 0;

			if (east != west) {
				shapeTouches &= ~(cardinalBit[Cardinal::EAST] | cardinalBit[Cardinal::WEST]);
				shapeTouches |= (east ? cardinalBit[Cardinal::WEST] : 0) | (west ? cardinalBit[Cardinal::EAST] : 0);
			}
		}
		if (vflipped) {
			bool north = (shapeTouches & cardinalBit[Cardinal::NORTH]) > 0;
			bool south = (shapeTouches & cardinalBit[Cardinal::SOUTH]) > 0;

			if (north != south) {
				shapeTouches &= ~(cardinalBit[Cardinal::NORTH] | cardinalBit[Cardinal::SOUTH]);
				shapeTouches |= (north ? cardinalBit[Cardinal::SOUTH] : 0) | (south ? cardinalBit[Cardinal::NORTH] : 0);
			}
		}
	}
};

struct SurfaceMaterial {
	float velocity = 0.f;
};

struct TileMaterial {

	const SurfaceMaterial& getSurface(Cardinal side, Cardinal facing = Cardinal::NORTH) const {
		size_t ndx = (static_cast<int>(side) - static_cast<int>(facing)) % 4;
		return surfaces.at(ndx);
	}

	std::string typeName;
	std::array<SurfaceMaterial, 4> surfaces;
};

struct tile_id {
	struct tile_value {
		uint8_t y : 4 = 15u;
		uint8_t x : 4 = 15u;
	};

	tile_id() = default;
	constexpr tile_id(
		std::unsigned_integral auto x, 
		std::unsigned_integral auto y) 
	{
		if (x > 15u || y > 15u) {
			reset();
		}
		else {
			pos.x = x;
			pos.y = y;
		}
	}

	constexpr tile_id(Vec2u v) {
		if (v.x > 15u || v.y > 15u) {
			reset();
		}
		else {
			pos.x = v.x;
			pos.y = v.y;
		}
	}

	constexpr tile_id(tile_value id) {
		pos = id;
	}

	constexpr tile_id(uint8_t id) {
		pos = std::bit_cast<tile_value>(id);
	}

	constexpr tile_id& operator= (Vec2u v) {
		pos.x = v.x;
		pos.y = v.y;
		return *this;
	}
	constexpr tile_id& operator= (uint8_t id) {
		pos = std::bit_cast<tile_value>(id);
		return *this;
	}

	constexpr Vec2u to_vec() const {
		return Vec2u{ pos.x, pos.y };
	}
	constexpr uint8_t to_byte() const {
		return std::bit_cast<uint8_t>(pos);
	}

	constexpr void reset() {
		pos = std::bit_cast<tile_value>(UINT8_MAX);
	}

	constexpr bool valid() const {
		return std::bit_cast<uint8_t>(pos) != UINT8_MAX;
	}

	constexpr operator bool() const {
		return std::bit_cast<uint8_t>(pos) != UINT8_MAX;
	}
	constexpr operator Vec2u() const {
		return to_vec();
	}
	constexpr operator uint8_t() const {
		return to_byte();
	}

	constexpr bool operator== (tile_id id) const {
		return std::bit_cast<uint8_t>(id.pos) == std::bit_cast<uint8_t>(pos);
	}
	constexpr bool operator!= (tile_id id) const {
		return std::bit_cast<uint8_t>(id.pos) != std::bit_cast<uint8_t>(pos);
	}

	tile_value pos;
};

class Tile {
	constexpr static int SAME_TILESET = -1;
public:
	tile_id pos;
	TileShape shape;
	Cardinal matFacing = Cardinal::NORTH;

	// tile next reference
	tile_id next_offset = 0u;
	int next_tileset = SAME_TILESET;

	const TilesetAsset* origin = nullptr;

	bool has_next_tileset() const {
		return next_tileset != SAME_TILESET;
	}

	static const TileMaterial standardMat;
	static void addMaterial(const TileMaterial& mat);
	static const TileMaterial& getMaterial(std::string typeName);

};


}