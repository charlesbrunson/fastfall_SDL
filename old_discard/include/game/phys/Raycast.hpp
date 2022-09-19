#pragma once

#include "fastfall/util/math.hpp"

#include "collider_coretypes/ColliderSurface.hpp"
#include "ColliderRegion.hpp"
//#include "../CollisionManager.hpp"

#include "../GameContext.hpp"

#include <optional>

namespace ff {

constexpr static float RAY_MAX_DIST = 500.f;

struct RaycastHit {
	float distance;
	Vec2f origin;
	Vec2f impact;
	const ColliderRegion* region;
	const ColliderSurface* surface;

	Linef get_line() const { return {origin, impact}; };
};

std::optional<RaycastHit> raycast(GameContext context, const Vec2f& origin, Cardinal direction, float dist = 100.f, float backoff = -1.f);

}
