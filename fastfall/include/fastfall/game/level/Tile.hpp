#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/cardinal.hpp"

namespace ff {

class TilesetAsset;

static constexpr unsigned TILE_TYPE_COUNT = 13U;

class TileShape {
public:
	enum class Type {
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
	unsigned shapeTouches : 4; // contains Cardinal bits
	bool hflipped = false;
	bool vflipped = false;
private:

	using TilePrototype = std::pair<TileShape::Type, unsigned>;
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

class Tile {
	constexpr static int NOT_ANIMATED = 0;
	constexpr static int SAME_TILESET = -1;

public:
	Vec2u pos;
	TileShape shape;
	const TilesetAsset* origin = nullptr;

	// for animated tiles
	Vec2i next_offset;
	unsigned int durationMS = NOT_ANIMATED;
	int next_tileset = SAME_TILESET; // index of name in tilesetReferences of TilesetAsset

	bool has_animation() {
		return durationMS != NOT_ANIMATED;
	}
	bool has_next_tileset() {
		return next_tileset != SAME_TILESET;
	}
};

}