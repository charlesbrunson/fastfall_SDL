#pragma once

#include "fastfall/resource/asset/TilesetAsset.hpp"

#include "ColliderQuad.hpp"

namespace ff {

struct ColliderTile {

	ColliderTile() = default;
	ColliderTile(Vec2i _pos, TileShape _shape) :
		position(_pos),
		shape(_shape)
	{

	};
	ColliderTile(Vec2i _pos, TileShape _shape, const TileMaterial* _mat, Cardinal _matface) :
		position(_pos),
		shape(_shape),
		mat(_mat),
		matFacing(_matface)
	{

	};

	ColliderQuad toQuad(int id = -1) const;

	Vec2i position;
	TileShape shape;

	const TileMaterial* mat = nullptr;
	Cardinal matFacing = Cardinal::NORTH;
};

}