#include "fastfall/game/phys/Raycast.hpp"

#include "fastfall/util/cardinal.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/game/CollisionManager.hpp"

namespace ff {

void debugDrawRaycast(std::optional<RaycastHit> result, Linef raycastLine) {

	if (result.has_value()) {
		auto& rayX = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_RAYCAST>(Primitive::LINES, 8);
		rayX[0].color = Color::Red;
		rayX[1].color = Color::Red;
		rayX[2].color = Color::Red;
		rayX[3].color = Color::Red;
		rayX[4].color = Color::Red;
		rayX[5].color = Color::Red;
		rayX[6].color = Color::Red;
		rayX[7].color = Color::Red;
		rayX[0].pos = result.value().impact + Vec2f(-2.f, -2.f);
		rayX[1].pos = result.value().impact + Vec2f(2.f, 2.f);
		rayX[2].pos = result.value().impact + Vec2f(2.f, -2.f);
		rayX[3].pos = result.value().impact + Vec2f(-2.f, 2.f);
		rayX[4].pos = result.value().origin;
		rayX[5].pos = result.value().impact;
		rayX[6].pos = result.value().surface->surface.p1 + result.value().region->getPosition();
		rayX[7].pos = result.value().surface->surface.p2 + result.value().region->getPosition();
	}
	else {
		auto& empty = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_RAYCAST>(Primitive::LINE_LOOP, 4);
		empty[0].color = Color::Red;
		empty[1].color = Color::Red;
		empty[2].color = Color::Red;
		empty[3].color = Color::Red;
		//empty[4].color = sf::Color::Red;
		empty[0].pos = raycastLine.p2 + Vec2f(-2.f, -2.f);
		empty[1].pos = raycastLine.p2 + Vec2f(2.f, -2.f);
		empty[2].pos = raycastLine.p2 + Vec2f(2.f, 2.f);
		empty[3].pos = raycastLine.p2 + Vec2f(-2.f, 2.f);
		//empty[4].position = raycastLine.p2 + Vec2f(-2.f, -2.f);

		auto& line = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_RAYCAST>(Primitive::LINES, 2);
		line[0].color = Color::Red;
		line[1].color = Color::Red;
		line[0].pos = raycastLine.p1;
		line[1].pos = raycastLine.p2;
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

std::optional<RaycastHit> raycastQuad(const ColliderRegion* region, const ColliderQuad* quad, const Linef& raycastLine, float backoff) {

	std::optional<RaycastHit> result{};

	for (const auto& surf : quad->surfaces) {
		if (!surf.hasSurface ||
			(math::dot(math::vector(surf.collider.surface).lefthand(), math::vector(raycastLine)) > 0.f))
			continue;

		Linef surface{ surf.collider.surface.p1 + region->getPosition(), surf.collider.surface.p2 + region->getPosition() };

		bool is_between = math::is_vertical(raycastLine) ?
			raycastLine.p1.x == math::clamp(raycastLine.p1.x, surface.p1.x, surface.p2.x) :
			raycastLine.p1.y == math::clamp(raycastLine.p1.y, surface.p1.y, surface.p2.y);

		if (is_between) {

			Vec2f intersect = math::intersection(raycastLine, surface);
			float dot = math::dot((intersect - (raycastLine.p1 - math::vector(raycastLine))), math::vector(raycastLine));

			if (!std::isnan(intersect.x) && !std::isnan(intersect.y) && (dot >= 0.f)) {

				float nDist = 0.f;

				auto dir = vecToCardinal(math::vector(raycastLine)).value();
				switch (dir) {
				case Cardinal::NORTH: nDist = raycastLine.p1.y - intersect.y; break;
				case Cardinal::SOUTH: nDist = intersect.y - raycastLine.p1.y;  break;
				case Cardinal::EAST:  nDist = intersect.x - raycastLine.p1.x; break;
				case Cardinal::WEST:  nDist = raycastLine.p1.x - intersect.x; break;
				}

				if ((!result.has_value() || nDist < result->distance) && nDist >= backoff) {

					result = compareHits(result,
						RaycastHit{
							.distance = nDist,
							.origin = raycastLine.p1,
							.impact = intersect,
							.region = region,
							.surface = &surf.collider
						});

				}
			}
		}
	}
	return result;
}

std::optional<RaycastHit> raycastRegion(ColliderRegion* region, const Rectf& raycastArea, const Linef& raycastLine, float backoff) {

	std::vector<std::pair<Rectf, const ColliderQuad*>> buffer;

	region->getQuads(raycastArea, buffer);

	std::optional<RaycastHit> result{};

	for (auto& [area, quad] : buffer) {
		result = compareHits(result, raycastQuad(region, quad, raycastLine, backoff));
	}
	return result;
}


std::optional<RaycastHit> raycast(CollisionContext phys_context, const Vec2f& origin, Cardinal direction, float dist, float backoff) {

	float distance = abs(dist) > RAY_MAX_DIST ? RAY_MAX_DIST : abs(dist);
	float backoff_ = -abs(backoff);

	Vec2f rayVec{ cardinalToVec2f(direction) * distance };
	Linef raycastLine{ origin, origin + rayVec };
	Rectf raycastArea{ origin, Vec2f{} };

	switch (direction) {
	case Cardinal::NORTH:
		raycastArea.top -= distance;
		raycastArea.height = distance;
		break;
	case Cardinal::EAST:
		raycastArea.width = distance;
		break;
	case Cardinal::SOUTH:
		raycastArea.height = distance;
		break;
	case Cardinal::WEST:
		raycastArea.left -= distance;
		raycastArea.width = distance;
		break;
	}

	std::vector<std::pair<Rectf, const ColliderQuad*>> buffer;

	//const auto* colliders = phys_context.get().getColliders();
	const auto& colliders = phys_context.get().get_colliders();

	std::optional<RaycastHit> result{};

	/*
	for (std::weak_ptr<ColliderRegion> region_wptr : colliders->colliders_free) {
		if (auto region = region_wptr.lock()) {
			result = compareHits(result, raycastRegion(region.get(), raycastArea, raycastLine, backoff_));
		}
	}
	for (std::weak_ptr<ColliderRegion> region_wptr : colliders->colliders_attached) {
		if (auto region = region_wptr.lock()) {
			result = compareHits(result, raycastRegion(region.get(), raycastArea, raycastLine, backoff_));
		}
	}
	*/
	for (auto& region_ptr : colliders) {
		result = compareHits(result, raycastRegion(region_ptr.get(), raycastArea, raycastLine, backoff_));
	}

	if (result.has_value() && result->distance >= distance) {
		result = std::nullopt;
	}

	if (debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_RAYCAST))
		debugDrawRaycast(result, raycastLine);

	return result;
}

}