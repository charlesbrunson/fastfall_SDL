#include "fastfall/game/phys/Raycast.hpp"

#include "fastfall/util/direction.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/game/World.hpp"

namespace ff {

void debugDrawRaycast(std::optional<RaycastHit> result, Linef raycastLine) {

    auto color = result.has_value() ? Color::Green : Color::Red;
	if (result.has_value()) {
		const auto& hit = result.value();

		auto& rayX = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_RAYCAST>(Primitive::LINES, 8);

		rayX[0].color = color;
		rayX[1].color = color;
		rayX[2].color = color;
		rayX[3].color = color;
		rayX[4].color = color;
		rayX[5].color = color;
		rayX[6].color = color;
		rayX[7].color = color;

		rayX[0].pos = hit.impact + Vec2f(-2.f, -2.f);
		rayX[1].pos = hit.impact + Vec2f(2.f, 2.f);
		rayX[2].pos = hit.impact + Vec2f(2.f, -2.f);
		rayX[3].pos = hit.impact + Vec2f(-2.f, 2.f);
		rayX[4].pos = hit.origin;
		rayX[5].pos = hit.impact;
		rayX[6].pos = hit.surface->surface.p1 + result.value().region->getPosition();
		rayX[7].pos = hit.surface->surface.p2 + result.value().region->getPosition();
	}
	else {
		auto& empty = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_RAYCAST>(Primitive::LINES, 10);
		empty[0].color = color;
		empty[1].color = color;
		empty[2].color = color;
		empty[3].color = color;
        empty[4].color = color;
        empty[5].color = color;
        empty[6].color = color;
        empty[7].color = color;
        empty[8].color = color;
        empty[9].color = color;

		empty[0].pos = raycastLine.p2 + Vec2f(-2.f, -2.f);
        empty[1].pos = raycastLine.p2 + Vec2f(2.f, -2.f);
		empty[2].pos = raycastLine.p2 + Vec2f(2.f, -2.f);
        empty[3].pos = raycastLine.p2 + Vec2f(2.f, 2.f);
		empty[4].pos = raycastLine.p2 + Vec2f(2.f, 2.f);
        empty[5].pos = raycastLine.p2 + Vec2f(-2.f, 2.f);
		empty[6].pos = raycastLine.p2 + Vec2f(-2.f, 2.f);
        empty[7].pos = raycastLine.p2 + Vec2f(-2.f, -2.f);
        empty[8].pos = raycastLine.p1;
        empty[9].pos = raycastLine.p2;
	}
}



std::optional<RaycastHit> compareHits(const std::optional<RaycastHit>& lhs, const std::optional<RaycastHit>& rhs) noexcept {

	if (!lhs.has_value() && !rhs.has_value()) {
		return std::nullopt;
	}
	else if (lhs.has_value() && !rhs.has_value()) {
		return lhs;
	}
	else if (!lhs.has_value() && rhs.has_value()) {
		return rhs;
	}
	else if (lhs.value().distance < rhs.value().distance) {
		return lhs;
	}
	else {
		return rhs;
	}
}

std::optional<RaycastHit> raycast_surface(const ColliderRegion& region, const ColliderSurface* surf, const Linef& raycastLine, float backoff) {
    if (!surf) return {};

    Linef surface  = math::shift(surf->surface, region.getPosition());
    Vec2f normal   = math::vector(surface).lefthand().unit();

    if (math::dot(math::vector(raycastLine), normal) < 0.f)
    {
        Vec2f unit      = math::vector(raycastLine).unit();
        Linef line      = { raycastLine.p1 + unit * backoff, raycastLine.p2 };
        Vec2f intersect = math::intersection(line, surface);
        bool  forwards  = math::dot(unit, raycastLine.p1 - line.p1) >= 0;
        float distance  = math::dist(raycastLine.p1, intersect) * (forwards ? 1.f : -1.f);

        if (math::line_has_point(line, intersect, 0.01f)
            && math::line_has_point(surface,  intersect, 0.01f)
            && distance >= backoff)
        {
            return RaycastHit{
                .distance = distance,
                .origin   = raycastLine.p1,
                .impact   = intersect,
                .region   = &region,
                .surface  = surf
            };
        }
    }
    return {};
}

std::optional<RaycastHit> raycast_quad_diag(const ColliderRegion& region, const ColliderQuad* quad, const Linef& raycastLine, float backoff) {
    std::optional<RaycastHit> result{};

    auto bounds = quad->get_bounds();
    if (!bounds || !bounds->contains(math::shift(raycastLine, -region.getPosition())))
        return result;

    for (const auto& surf : quad->surfaces) {
        if (!surf.hasSurface ||
            (math::dot(math::vector(surf.collider.surface).lefthand(), math::vector(raycastLine)) > 0.f))
            continue;

        for (auto dir : direction::cardinals) {
            result = compareHits(result, raycast_surface(region, quad->getSurface(dir), raycastLine, backoff));
        }
    }

    return result;
}

std::optional<RaycastHit> raycast_quad_ortho(const ColliderRegion& region, const ColliderQuad* quad, const Linef& raycastLine, float backoff) {

	std::optional<RaycastHit> result{};

    if (!quad->get_bounds()->contains(raycastLine))
        return result;

	for (const auto& surf : quad->surfaces) {
		if (!surf.hasSurface ||
			(math::dot(math::vector(surf.collider.surface).lefthand(), math::vector(raycastLine)) > 0.f))
			continue;

        auto surface = math::shift(surf.collider.surface, region.getPosition());

        bool is_between = math::is_vertical(raycastLine) ?
         	raycastLine.p1.x >= std::min(surface.p1.x, surface.p2.x) && raycastLine.p1.x <= std::max(surface.p1.x, surface.p2.x):
         	raycastLine.p1.y >= std::min(surface.p1.y, surface.p2.y) && raycastLine.p1.y >= std::max(surface.p1.y, surface.p2.y);

		if (is_between) {

			Vec2f intersect = math::intersection(raycastLine, surface);
			float dot = math::dot((intersect - (raycastLine.p1 - math::vector(raycastLine))), math::vector(raycastLine));

			if (!std::isnan(intersect.x) && !std::isnan(intersect.y) && (dot >= 0.f)) {

				float nDist = 0.f;

				auto dir = direction::from_vector(math::vector(raycastLine)).value();
				switch (dir) {
				case Cardinal::N: nDist = raycastLine.p1.y - intersect.y; break;
				case Cardinal::S: nDist = intersect.y - raycastLine.p1.y; break;
				case Cardinal::E: nDist = intersect.x - raycastLine.p1.x; break;
				case Cardinal::W: nDist = raycastLine.p1.x - intersect.x; break;
				}

				if ((!result.has_value() || nDist < result->distance) && nDist >= backoff) {
					result = compareHits(result,
						RaycastHit{
							.distance = nDist,
							.origin   = raycastLine.p1,
							.impact   = intersect,
							.region   = &region,
							.surface  = &surf.collider
						});

				}
			}
		}
	}
	return result;
}

std::optional<RaycastHit> raycastRegion(const ColliderRegion& region, const Linef& raycastLine, float backoff) {

	// std::vector<const ColliderQuad*> buffer;
    // for (auto& quad : region.in_line(raycastLine)) {
    //     buffer.push_back(&quad);
    // }

    bool is_ortho = direction::from_vector(math::vector(raycastLine)).has_value();
	std::optional<RaycastHit> result{};
	for (auto& quad : region.in_line(raycastLine)) {
		result = compareHits(result,
                             (is_ortho
                             ? raycast_quad_ortho(region, &quad, raycastLine, backoff)
                             : raycast_quad_diag(region, &quad, raycastLine, backoff)
                             ));
	}
	return result;
}

std::optional<RaycastHit> raycast(const poly_id_map<ColliderRegion>& regions, Linef path, float backoff) {
    float distance = math::dist(path);
    backoff = -abs(backoff);

    std::vector<std::pair<Rectf, const ColliderQuad*>> buffer;
    std::optional<RaycastHit> result{};

    for (auto [id, region_ptr] : regions) {
        result = compareHits(result, raycastRegion(*region_ptr, path, backoff));
    }

    if (result.has_value() && result->distance >= distance) {
        result = std::nullopt;
    }

    if (debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_RAYCAST)) {
        debugDrawRaycast(result, path);
    }

    return result;
}

}
