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

    [[nodiscard]]
	inline Linef line() const { return { origin, impact }; };
};

std::optional<RaycastHit> raycast(const World& w, Linef path, float backoff = -1.f);
// std::optional<RaycastHit> raycast(std::ranges::range<ColliderRegion> regions, Linef path, float backoff = -1.f);

//std::optional<RaycastHit> raycast(const poly_id_map<ColliderRegion>& regions, const Vec2f& origin, Cardinal direction, float distance, float backoff = -1.f) {
//    return raycast(regions, Linef{ origin, origin + direction::to_vector<float>(direction) * distance }, backoff);
//}
//
//std::optional<RaycastHit> raycast(const poly_id_map<ColliderRegion>& regions, Vec2f start, Vec2f end, float backoff = -1.f) {
//    return raycast(regions, Linef{ start, end }, backoff);
//}

}
