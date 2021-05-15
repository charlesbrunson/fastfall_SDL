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

	ColliderQuad toQuad() const;

	Vec2i position;
	TileShape shape;
};

}