#include "fastfall/game/phys/collider_coretypes/ColliderTile.hpp"

#include "fastfall/engine/config.hpp"
#include "fastfall/util/log.hpp"

namespace ff {

constexpr unsigned HALFTILE = TILESIZE / 2;

//constexpr std::map<TileShape::Type, ColliderQuad::SurfaceArray> surfacePrototypes

using TypeSurfaceArray = std::pair<TileShape::Type, ColliderQuad::SurfaceArray>;
constexpr std::array<TypeSurfaceArray, TILE_TYPE_COUNT> surfacePrototypes
{
	TypeSurfaceArray{TileShape::Type::SOLID, {
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        0),        Vec2f(TILESIZE, 0)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, 0),        Vec2f(TILESIZE, TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, TILESIZE), Vec2f(0,        TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        TILESIZE), Vec2f(0,        0)) }},
	}},
	{TileShape::Type::HALF, {
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        HALFTILE), Vec2f(TILESIZE, HALFTILE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, HALFTILE), Vec2f(TILESIZE, TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, TILESIZE), Vec2f(0,        TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        TILESIZE), Vec2f(0,        HALFTILE)) }}
	}},
	{TileShape::Type::HALFVERT, {
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        0),        Vec2f(HALFTILE, 0)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(HALFTILE, 0),        Vec2f(HALFTILE, TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(HALFTILE, TILESIZE), Vec2f(0,        TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        TILESIZE), Vec2f(0,        0)) }}
	}},
	{TileShape::Type::ONEWAY, {
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        0),        Vec2f(TILESIZE, 0)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, 0),        Vec2f(TILESIZE, TILESIZE)) }, false},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, TILESIZE), Vec2f(0,        TILESIZE)) }, false},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        TILESIZE), Vec2f(0,        0)) },        false},
	}},
	{TileShape::Type::ONEWAYVERT, {
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        0),        Vec2f(TILESIZE, 0)) },        false},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, 0),        Vec2f(TILESIZE, TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, TILESIZE), Vec2f(0,        TILESIZE)) }, false},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        TILESIZE), Vec2f(0,        0)) },        false},
	}},
	{TileShape::Type::SLOPE, {
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        TILESIZE), Vec2f(TILESIZE, 0)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, 0),        Vec2f(TILESIZE, TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, TILESIZE), Vec2f(0,        TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, TILESIZE), Vec2f(0,        TILESIZE)) }, false}
	}},
	{TileShape::Type::SHALLOW1, {
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        TILESIZE), Vec2f(TILESIZE, HALFTILE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, HALFTILE), Vec2f(TILESIZE, TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, TILESIZE), Vec2f(0,        TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, TILESIZE), Vec2f(0,        TILESIZE)) }, false}
	}},
	{TileShape::Type::SHALLOW2, {
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        HALFTILE), Vec2f(TILESIZE, 0)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, 0),        Vec2f(TILESIZE, TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, TILESIZE), Vec2f(0,        TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        TILESIZE), Vec2f(0,        HALFTILE)) }}
	}},

	{TileShape::Type::STEEP1, {
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(HALFTILE, 0),        Vec2f(TILESIZE, 0)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, 0),        Vec2f(TILESIZE, TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, TILESIZE), Vec2f(0,        TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        TILESIZE), Vec2f(HALFTILE, 0)) }}
	}},
	{TileShape::Type::STEEP2, {
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, 0),        Vec2f(TILESIZE, 0)) },        false},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, 0),        Vec2f(TILESIZE, TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, TILESIZE), Vec2f(HALFTILE, TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(HALFTILE, TILESIZE), Vec2f(TILESIZE, 0)) }}
	}},
	{TileShape::Type::EMPTY, {
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        0),        Vec2f(TILESIZE, 0)) },        false},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, 0),        Vec2f(TILESIZE, TILESIZE)) }, false},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, TILESIZE), Vec2f(0,        TILESIZE)) }, false},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        TILESIZE), Vec2f(0,        0)) },        false},
	}},
	{TileShape::Type::LEVELBOUNDARY, {
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        0),        Vec2f(TILESIZE, 0)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, 0),        Vec2f(TILESIZE, TILESIZE)) }, false},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, TILESIZE), Vec2f(0,        TILESIZE)) }, false},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        TILESIZE), Vec2f(0,        0)) },        false},
	}},
	{TileShape::Type::LEVELBOUNDARY_WALL, {
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        0),        Vec2f(TILESIZE, 0)) },        false},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, 0),        Vec2f(TILESIZE, TILESIZE)) }},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(TILESIZE, TILESIZE), Vec2f(0,        TILESIZE)) }, false},
		ColliderQuad::QuadSurface{ColliderSurface{ Linef(Vec2f(0,        TILESIZE), Vec2f(0,        0)) },        false},
	}}
};



ColliderQuad ColliderTile::toQuad(int id) const {

	//ColliderQuad q{ surfacePrototypes.at(shape.type) };

	ColliderQuad q{
		std::find_if(surfacePrototypes.begin(), surfacePrototypes.end(),
		[this](const TypeSurfaceArray& element) {
				return element.first == shape.type;
		})->second
	};


	//do transform
	for (auto& colSurf : q.surfaces) {
		Linef& surfLine = colSurf.collider.surface;

		const static auto flip = [](float& i) {
			i = -i + TILESIZE_F;
		};

		if (shape.hflipped) {
			flip(surfLine.p1.x);
			flip(surfLine.p2.x);
		}
		if (shape.vflipped) {
			flip(surfLine.p1.y);
			flip(surfLine.p2.y);
		}

		//keep surface lines going clockwise
		if ((shape.vflipped || shape.hflipped) && !(shape.vflipped && shape.hflipped))
			std::swap(surfLine.p1, surfLine.p2);

		surfLine.p1 += position * TILESIZE_F;
		surfLine.p2 += position * TILESIZE_F;
	}

	auto swapFacing = [&q](Cardinal dir) {
		auto& lhs = q.surfaces[dir];
		auto& rhs = q.surfaces[cardinalOpposite(dir)];


		if ((lhs.hasSurface) && (rhs.hasSurface)) {
			std::swap(lhs.collider, rhs.collider);
		}
		else if (lhs.hasSurface) {
			rhs = lhs;
			lhs.hasSurface = false;
		}
		else if (rhs.hasSurface) {
			lhs = rhs;
			rhs.hasSurface = false;
		}
	};

	if (shape.hflipped) {
		swapFacing(Cardinal::EAST);
	}
	if (shape.vflipped) {
		swapFacing(Cardinal::NORTH);
	}

	if (shape.type == TileShape::Type::ONEWAY) {
		q.hasOneWay = true;
		q.oneWayDir = !shape.vflipped ? Cardinal::NORTH : Cardinal::SOUTH;
	}
	else if (shape.type == TileShape::Type::ONEWAYVERT) {
		q.hasOneWay = true;
		q.oneWayDir = !shape.hflipped ? Cardinal::EAST : Cardinal::WEST;
	}
	else if (shape.type == TileShape::Type::LEVELBOUNDARY) {
		q.hasBoundary = true;
		q.oneWayDir = !shape.vflipped ? Cardinal::NORTH : Cardinal::SOUTH;
	}
	else if (shape.type == TileShape::Type::LEVELBOUNDARY_WALL) {
		q.hasBoundary = true;
		q.oneWayDir = !shape.hflipped ? Cardinal::EAST : Cardinal::WEST;
	}

	q.setID(id);

	// material
	q.material = mat;
	q.matFacing = matFacing;

	return q;
}

}
