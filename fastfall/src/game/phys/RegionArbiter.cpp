#include "fastfall/game/phys/RegionArbiter.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/render/DebugDraw.hpp"

namespace ff {

void RegionArbiter::updateRegion(Rectf bounds) {

	currQuads.clear();
	collider_->get_quads_in_rect(bounds, currQuads);

	// check for stale (out of bounds) quads to remove
	for (auto& [_, arb] : quadArbiters) {
		if (!bounds.touches(arb.quad_bounds)) 
		{
			arb.stale = true;
		}
	}

	// create arbiters and/or update arbiters 
	for (auto& q : currQuads) {
		if (!q.second->hasAnySurface())
			continue;

		auto iter = quadArbiters.lower_bound(q.second);

		if (iter == quadArbiters.end() || iter->first != q.second) {
			// just entered this quad
			iter = quadArbiters.insert(iter, { q.second, Arbiter{ collidable_, q.second, collider_ } });
		}
		iter->second.stale = false;
	}

	std::erase_if(quadArbiters, [](const auto& pair) { return pair.second.stale; });
}

}