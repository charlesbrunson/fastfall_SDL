#pragma once

#include "fastfall/util/math.hpp"

#include "fastfall/game/phys/collider_coretypes/ColliderSurface.hpp"
#include "fastfall/game/phys/ColliderRegion.hpp"

#include <optional>
#include <ranges>

namespace ff {

class World;

struct RaycastHit {
	float distance;
	Vec2f origin;
	Vec2f impact;
	const ColliderRegion* region;
	const ColliderSurface* surface;

    [[nodiscard]] Linef line() const { return { origin, impact }; };
};

std::optional<RaycastHit> raycast(const World& w, Linef path, float backoff = -1.f);

}
