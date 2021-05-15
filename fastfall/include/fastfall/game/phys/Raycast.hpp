#pragma once

#include "fastfall/util/math.hpp"

#include "fastfall/game/phys/collider_coretypes/ColliderSurface.hpp"
#include "fastfall/game/phys/ColliderRegion.hpp"
//#include "../CollisionManager.hpp"

#include "fastfall/game/GameContext.hpp"

#include <optional>

namespace ff {

constexpr static float RAY_MAX_DIST = 500.f;

struct RaycastHit {
	float distance;
	Vec2f origin;
	Vec2f impact;
	const ColliderRegion* region;
	const ColliderSurface* surface;
};

std::optional<RaycastHit> raycast(CollisionContext phys_context, const Vec2f& origin, Cardinal direction, float dist = 100.f, float backoff = -1.f);

}