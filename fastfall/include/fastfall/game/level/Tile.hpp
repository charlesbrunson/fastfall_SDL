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

struct TileID {
	uint8_t y : 4 = 15u;
	uint8_t x : 4 = 15u;

	TileID() = default;
	constexpr TileID(
		std::unsigned_integral auto _x, 
		std::unsigned_integral auto _y) 
	{
		if (_x > 15u || _y > 15u) {
			reset();
		}
		else {
			x = _x;
			y = _y;
		}
	}

	explicit constexpr TileID(Vec2u v) {
		if (v.x > 15u || v.y > 15u) {
			reset();
		}
		else {
			x = v.x;
			y = v.y;
		}
	}

	explicit constexpr TileID(uint8_t id) {
		*this = std::bit_cast<uint8_t>(id);
	}

	constexpr TileID& operator= (uint8_t id) {
		y = id >> 4;
		x = id & 15u;
		return *this;
	}

	constexpr Vec2u to_vec() const {
		return Vec2u{ x, y };
	}
	constexpr uint8_t to_byte() const {
		return std::bit_cast<uint8_t>(*this);
	}

	constexpr void reset() {
		*this = std::bit_cast<uint8_t>(UINT8_MAX);
	}

	constexpr bool valid() const {
		return std::bit_cast<uint8_t>(*this) != UINT8_MAX;
	}

	constexpr operator bool() const {
		return std::bit_cast<uint8_t>(*this) != UINT8_MAX;
	}
	constexpr operator Vec2u() const {
		return to_vec();
	}
	constexpr operator uint8_t() const {
		return to_byte();
	}

	constexpr bool operator== (TileID id) const {
		return std::bit_cast<uint8_t>(id) == std::bit_cast<uint8_t>(*this);
	}
	constexpr bool operator!= (TileID id) const {
		return std::bit_cast<uint8_t>(id) != std::bit_cast<uint8_t>(*this);
	}

	constexpr TileID operator+ (TileID rhs) {
		TileID r = *this;
		r.x += rhs.x;
		r.y += rhs.y;
		return r;
	}

};

class Tile {
	constexpr static int SAME_TILESET = -1;
public:
	TileID pos;
	TileShape shape;
	Cardinal matFacing = Cardinal::NORTH;

	// tile next reference
	TileID next_offset = TileID{ 0u };
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