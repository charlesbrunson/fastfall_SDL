#pragma once

#include "fastfall/util/math.hpp"

#include "fastfall/game/phys/collider_coretypes/ColliderSurface.hpp"
#include "fastfall/game/phys/ColliderRegion.hpp"

#include <optional>

namespace ff {

class World;

constexpr static float RAY_MAX_DIST = 500.f;

struct RaycastHit {
	float distance;
	Vec2f origin;
	Vec2f impact;
	const ColliderRegion* region;
	const ColliderSurface* surface;

	Linef get_line() const { return {origin, impact}; };
};

std::optional<RaycastHit> raycast(World& world, const Vec2f& origin, Cardinal direction, float dist = 100.f, float backoff = -1.f);

}
