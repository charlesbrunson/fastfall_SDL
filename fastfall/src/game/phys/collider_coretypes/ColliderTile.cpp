#include "fastfall/game/phys/collider_coretypes/ColliderTile.hpp"

#include "fastfall/engine/config.hpp"
#include "fastfall/util/log.hpp"

namespace ff {

constexpr unsigned HALFTILE = TILESIZE / 2;

//constexpr std::map<TileShape::Type, ColliderQuad::SurfaceArray> surfacePrototypes

//using TypeSurfaceArray = std::pair<TileShape::Type, cardinal_array<ColliderQuad::QuadSurface>>;

struct TypeSurfaceArray
{
	TileShape::Type type;
	cardinal_array<ColliderQuad::QuadSurface> surfaces;
};

constexpr TypeSurfaceArray make_prototype(TileShape::Type shape, std::array<Vec2f, 4> points, unsigned has_surface_bits = UINT_MAX)
{
	TypeSurfaceArray tsa;
	tsa.type = shape;

	for (int i = 0; i < 4; i++)
	{
		Cardinal dir = static_cast<Cardinal>(i);
		auto& surface = tsa.surfaces[dir];

		surface.collider.surface.p1 = points[i] * TILESIZE_F;
		surface.collider.surface.p2 = points[(i + 1) % 4] * TILESIZE_F;
		surface.hasSurface = (has_surface_bits & direction::to_bits(dir)) > 0;
	}
	return tsa;
}

constexpr std::array surfacePrototypes = {
	make_prototype(TileShape::Type::Solid,				{ Vec2f{0, 0},		{1, 0},		{1, 1},		{0, 1}	}),
	make_prototype(TileShape::Type::Half,				{ Vec2f{0, 0.5},	{1, 0.5},	{1, 1},		{0, 1}	}),
	make_prototype(TileShape::Type::HalfVert,			{ Vec2f{0, 0},		{0.5, 0},	{0.5, 1},	{0, 1}	}),
	make_prototype(TileShape::Type::Oneway,				{ Vec2f{0, 0},		{1, 0},		{1, 1},		{0, 1}	}, direction::to_bits(Cardinal::N)),
	make_prototype(TileShape::Type::OnewayVert,			{ Vec2f{0, 0},		{1, 0},		{1, 1},		{0, 1}	}, direction::to_bits(Cardinal::E)),
	make_prototype(TileShape::Type::Slope,				{ Vec2f{0, 1},		{1, 0},		{1, 1},		{0, 1}	}, direction::to_bits(Cardinal::N, Cardinal::E, Cardinal::S)),
	make_prototype(TileShape::Type::Shallow1,			{ Vec2f{0, 1},		{1, 0.5},	{1, 1},		{0, 1}	}, direction::to_bits(Cardinal::N, Cardinal::E, Cardinal::S)),
	make_prototype(TileShape::Type::Shallow2,			{ Vec2f{0, 0.5},	{1, 0},		{1, 1},		{0, 1}	}),
	make_prototype(TileShape::Type::Steep1,				{ Vec2f{0.5, 0},	{1, 0},		{1, 1},		{0, 1}	}),
	make_prototype(TileShape::Type::Steep2,				{ Vec2f{1, 0},		{1, 0},		{1, 1},		{0.5, 1}}, direction::to_bits(Cardinal::E, Cardinal::S, Cardinal::W)),
	make_prototype(TileShape::Type::Empty,				{ Vec2f{0, 0},		{1, 0},		{1, 1},		{0, 1}	}, direction::to_bits(Cardinal::N, Cardinal::E, Cardinal::S, Cardinal::W)),
	make_prototype(TileShape::Type::LevelBoundary,		{ Vec2f{0, 0},		{1, 0},		{1, 1},		{0, 1}	}, direction::to_bits(Cardinal::N)),
	make_prototype(TileShape::Type::LevelBoundary_Wall,	{ Vec2f{0, 0},		{1, 0},		{1, 1},		{0, 1}	}, direction::to_bits(Cardinal::E)),
};



ColliderQuad ColliderTile::toQuad(QuadID id) const {

	//ColliderQuad q{ surfacePrototypes.at(shape.type) };

	ColliderQuad q{
		std::find_if(surfacePrototypes.begin(), surfacePrototypes.end(),
		[this](const TypeSurfaceArray& element) {
				return element.type == shape.type;
		})->surfaces
	};


	//do transform
	for (auto& colSurf : q.surfaces) {
		Linef& surfLine = colSurf.collider.surface;

		const static auto flip = [](float& i) {
			i = -i + TILESIZE_F;
		};

		if (shape.flip_h) {
			flip(surfLine.p1.x);
			flip(surfLine.p2.x);
		}
		if (shape.flip_v) {
			flip(surfLine.p1.y);
			flip(surfLine.p2.y);
		}

		//keep surface lines going clockwise
		if ((shape.flip_v || shape.flip_h) && !(shape.flip_v && shape.flip_h))
			std::swap(surfLine.p1, surfLine.p2);

		surfLine.p1 += position * TILESIZE_F;
		surfLine.p2 += position * TILESIZE_F;
	}

	auto swapFacing = [&q](Cardinal dir) {
		auto& lhs = q.surfaces[dir];
		auto& rhs = q.surfaces[direction::opposite(dir)];


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

	if (shape.flip_h) {
		swapFacing(Cardinal::E);
	}
	if (shape.flip_v) {
		swapFacing(Cardinal::N);
	}

	if (shape.type == TileShape::Type::Oneway) {
		q.hasOneWay = true;
		q.oneWayDir = !shape.flip_v ? Cardinal::N : Cardinal::S;
	}
	else if (shape.type == TileShape::Type::OnewayVert) {
		q.hasOneWay = true;
		q.oneWayDir = !shape.flip_h ? Cardinal::E : Cardinal::W;
	}
	else if (shape.type == TileShape::Type::LevelBoundary) {
		q.hasBoundary = true;
		q.oneWayDir = !shape.flip_v ? Cardinal::N : Cardinal::S;
	}
	else if (shape.type == TileShape::Type::LevelBoundary_Wall) {
		q.hasBoundary = true;
		q.oneWayDir = !shape.flip_h ? Cardinal::E : Cardinal::W;
	}

	q.setID(id);

	// material
	q.material = mat;
	q.matFacing = matFacing;

	return q;
}

}
