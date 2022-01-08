#pragma once

#include "fastfall/game/level/TileID.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/util/direction.hpp"


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
		ONEWAYVERT,

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
		{Type::SOLID,				direction::to_bits(Cardinal::N, Cardinal::E, Cardinal::S, Cardinal::W)},
		{Type::HALF,				direction::to_bits(Cardinal::E, Cardinal::S, Cardinal::W)},
		{Type::HALFVERT,			direction::to_bits(Cardinal::N,                 Cardinal::S, Cardinal::W)},
		{Type::SLOPE,				direction::to_bits(Cardinal::E, Cardinal::S)},
		{Type::SHALLOW1,			direction::to_bits(Cardinal::E, Cardinal::S)},
		{Type::SHALLOW2,			direction::to_bits(Cardinal::E, Cardinal::S, Cardinal::W)},
		{Type::STEEP1,				direction::to_bits(Cardinal::N, Cardinal::E, Cardinal::S)},
		{Type::STEEP2,				direction::to_bits(Cardinal::E, Cardinal::S)},
		{Type::ONEWAY,				direction::to_bits(Cardinal::N)},
		{Type::ONEWAYVERT,			direction::to_bits(Cardinal::E)},

		{Type::LEVELBOUNDARY,		direction::to_bits(Cardinal::N)},
		{Type::LEVELBOUNDARY_WALL,	direction::to_bits(Cardinal::E)}
	};

	constexpr void init() {

		shapeTouches = std::find_if(tilePrototypes.begin(), tilePrototypes.end(), [this](const TilePrototype& element) {
			return element.first == type;
			})->second;

		// flip touch flags
		if (hflipped) {
			bool east = (shapeTouches & direction::to_bits(Cardinal::E)) > 0;
			bool west = (shapeTouches & direction::to_bits(Cardinal::W)) > 0;

			if (east != west) {
				shapeTouches &= ~(direction::to_bits(Cardinal::E, Cardinal::W));
				shapeTouches |= (east ? direction::to_bits(Cardinal::W) : 0) | (west ? direction::to_bits(Cardinal::E) : 0);
			}
		}
		if (vflipped) {
			bool north = (shapeTouches & direction::to_bits(Cardinal::N)) > 0;
			bool south = (shapeTouches & direction::to_bits(Cardinal::S)) > 0;

			if (north != south) {
				shapeTouches &= ~(direction::to_bits(Cardinal::N, Cardinal::S));
				shapeTouches |= (north ? direction::to_bits(Cardinal::S) : 0) | (south ? direction::to_bits(Cardinal::N) : 0);
			}
		}
	}
};

struct SurfaceMaterial {
	float velocity = 0.f;
};

struct AutoTileRule {
	enum class Type {
		N_A,
		No,
		Yes,
		Shape
	};

	Type type = Type::N_A;
	TileShape shape;
};

struct TileMaterial {

	const SurfaceMaterial& getSurface(Cardinal side, Cardinal facing = Cardinal::N) const {
		size_t ndx = (static_cast<int>(side) - static_cast<int>(facing)) % 4;
		return surfaces.at(ndx);
	}

	std::string typeName;
	std::array<SurfaceMaterial, 4> surfaces;
};


class Tile {
public:
	TileID pos;
	TileShape shape;
	Cardinal matFacing = Cardinal::N;

	// tile next reference
	TileID next_offset = TileID{ 0u };
	std::optional<unsigned> next_tileset = std::nullopt;

	const TilesetAsset* origin = nullptr;

	std::array<AutoTileRule, 4> autotile_card;
	std::array<AutoTileRule, 4> autotile_ord;

	bool has_next_tileset() const {
		return next_tileset.has_value();
	}

	static const TileMaterial standardMat;
	static void addMaterial(const TileMaterial& mat);
	static const TileMaterial& getMaterial(std::string typeName);

};


}