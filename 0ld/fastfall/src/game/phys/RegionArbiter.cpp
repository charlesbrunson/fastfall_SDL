#include "fastfall/game/phys/RegionArbiter.hpp"

#include "tracy/Tracy.hpp"

namespace ff {

void RegionArbiter::updateRegion(CollisionContext ctx, Rectf bounds)
{
    ZoneScoped;
	currQuads.clear();

    Vec2f deltap = ctx.collider->getDeltaPosition();
    bounds = math::rect_extend(bounds, deltap.x < 0.f ? Cardinal::W : Cardinal::E, abs(deltap.x));
    bounds = math::rect_extend(bounds, deltap.y < 0.f ? Cardinal::N : Cardinal::S, abs(deltap.y));

    for (auto quad : ctx.collider->in_rect(bounds)) {
        currQuads.push_back(quad.id);
    }

	// check for stale (out of bounds) quads to remove
	for (auto& [_, arb] : quadArbiters) {
        arb.stale = true;
	}

	// create arbiters and/or update arbiters 
	for (auto& qid : currQuads) {
        auto quad = ctx.collider->get_quad(qid);

		if (!quad->hasAnySurface())
			continue;

		auto iter = quadArbiters.lower_bound(qid);

		if (iter == quadArbiters.end() || iter->first != qid) {
			// just entered this quad
            auto collision_id = CollisionID{ collidable_id, collider_id, qid };
			iter = quadArbiters.emplace(qid, Arbiter{ ctx, collision_id }).first;
		}
		iter->second.stale = false;
	}

	std::erase_if(quadArbiters, [](const auto& pair) { return pair.second.stale; });
}

}