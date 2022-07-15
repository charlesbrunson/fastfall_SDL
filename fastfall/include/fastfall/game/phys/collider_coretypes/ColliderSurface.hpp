#pragma once

#include "fastfall/util/math.hpp"
#include "QuadID.hpp"

namespace ff {

struct ColliderSurfaceID
{
	QuadID quad;
	Cardinal dir;
};

class ColliderSurface {
public:
	Linef surface;
	Vec2f ghostp0;
	Vec2f ghostp3;

	ColliderSurfaceID id;

	std::optional<ColliderSurfaceID> prev;
	std::optional<ColliderSurfaceID> next;

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

		//r.g0virtual = g3virtual;
		//r.g3virtual = g0virtual;

		r.prev = next;
		r.next = prev;

		return r;
	}
};

}