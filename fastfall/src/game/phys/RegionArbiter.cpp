#include "fastfall/game/phys/RegionArbiter.hpp"

namespace ff {

void RegionArbiter::updateRegion(CollisionContext ctx, Rectf bounds) {

	currQuads.clear();
	ctx.collider->get_quads_in_rect(bounds, currQuads);

	// check for stale (out of bounds) quads to remove
	for (auto& [_, arb] : quadArbiters) {
		if (!bounds.touches(arb.quad_bounds)) 
		{
			arb.stale = true;
		}
	}

	// create arbiters and/or update arbiters 
	for (auto [rect, qid] : currQuads) {
        auto quad = ctx.collider->get_quad(qid);
		if (!quad->hasAnySurface())
			continue;

		auto iter = quadArbiters.lower_bound(qid);

		if (iter == quadArbiters.end() || iter->first != qid) {
			// just entered this quad
            auto collision_id = CollisionID{ collidable_id, collider_id, qid };
			iter = quadArbiters.insert(iter, { qid, Arbiter{ ctx, *quad, collision_id } });
		}
		iter->second.stale = false;
	}

	std::erase_if(quadArbiters, [](const auto& pair) { return pair.second.stale; });
}

}