#include "fastfall/game_v2/phys/Raycast.hpp"

#include "fastfall/util/direction.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/game_v2/World.hpp"

namespace ff {

void debugDrawRaycast(std::optional<RaycastHit> result, Linef raycastLine) {

	if (result.has_value()) {
		const auto& hit = result.value();

		auto& rayX = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_RAYCAST>(Primitive::LINES, 8);

		rayX[0].color = Color::Red;
		rayX[1].color = Color::Red;
		rayX[2].color = Color::Red;
		rayX[3].color = Color::Red;
		rayX[4].color = Color::Red;
		rayX[5].color = Color::Red;
		rayX[6].color = Color::Red;
		rayX[7].color = Color::Red;

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

	std::vector<std::pair<Rectf, QuadID>> buffer;

	region->get_quads_in_rect(raycastArea, buffer);

	std::optional<RaycastHit> result{};

	for (auto& [area, quad_id] : buffer) {
		result = compareHits(result, raycastQuad(region, region->get_quad(quad_id), raycastLine, backoff));
	}
	return result;
}


std::optional<RaycastHit> raycast(World& world, const Vec2f& origin, Cardinal direction, float dist, float backoff) {

	float distance = abs(dist) > RAY_MAX_DIST ? RAY_MAX_DIST : abs(dist);
	float backoff_ = -abs(backoff);

	Vec2f rayVec{ direction::to_vector<float>(direction) * distance };
	Linef raycastLine{ origin, origin + rayVec };
	Rectf raycastArea = math::rect_extend({origin, {}}, direction, distance);

	std::vector<std::pair<Rectf, const ColliderQuad*>> buffer;

	//const auto& colliders = *instance::phys_get_colliders(context);

	std::optional<RaycastHit> result{};

	for (auto& region_ptr : world.all<ColliderRegion>()) {
		result = compareHits(result, raycastRegion(region_ptr.get(), raycastArea, raycastLine, backoff_));
	}

	if (result.has_value() && result->distance >= distance) {
		result = std::nullopt;
	}

	if (debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_RAYCAST)) {
		debugDrawRaycast(result, raycastLine);
	}

	return result;
}

}
