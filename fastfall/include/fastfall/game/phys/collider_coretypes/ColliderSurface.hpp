#pragma once

#include "fastfall/util/math.hpp"

namespace ff {

struct ColliderSurfaceID
{
	int quad_id = -1;
	Cardinal dir = Cardinal::N;
};

class ColliderSurface {
public:
	Linef surface;
	Vec2f ghostp0;
	Vec2f ghostp3;

	bool g0virtual = true;
	bool g3virtual = true;

	ColliderSurfaceID id = {};

	std::optional<ColliderSurfaceID> prev_id = std::nullopt;
	std::optional<ColliderSurfaceID> next_id = std::nullopt;


	//const ColliderSurface* prev = nullptr;
	//const ColliderSurface* next = nullptr;

	Linef getGhostPrev() const {
		return { ghostp0, surface.p1 };
	}

	Linef getGhostNext() const {
		return { surface.p2, ghostp3 };
	}

	ColliderSurface reverse() const {
		ColliderSurface r;

		r.surface = surface.reverse();
		r.ghostp0 = ghostp3;
		r.ghostp3 = ghostp0;

		r.g0virtual = g3virtual;
		r.g3virtual = g0virtual;

		r.prev_id = next_id;
		r.next_id = prev_id;

		return r;
	}
};

}