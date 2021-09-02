#include "fastfall/game/phys/RegionArbiter.hpp"

#include "fastfall/util/log.hpp"

namespace ff {

void RegionArbiter::updateRegion(Rectf bounds) {

	currQuads.clear();
	collider_->getQuads(bounds, currQuads);

	// check for stale (out of bounds) quads to remove
	for (auto q = prevQuads.cbegin(); q != prevQuads.cend(); q++) {
		//if (!bounds.intersects(q->first)) {
		if (!bounds.touches(q->first)) { // fixes oneway collision?

			// just exited this quad
			//quadArbiters.erase(q->second);
			auto quad_iter = quadArbiters.find(q->second);
			if (quad_iter != quadArbiters.end()) {
				quad_iter->second.stale = true;
			}
		}
	}

	// create arbiters and/or update arbiters 
	for (auto& q : currQuads) {
		if (!q.second->hasAnySurface())
			continue;

		auto iter = quadArbiters.lower_bound(q.second);

		if (iter == quadArbiters.end() || iter->first != q.second) {
			// just entered this quad
			iter = quadArbiters.insert(iter, std::make_pair(q.second, Arbiter(collidable_, q.second, collider_)));
		}
		iter->second.stale = false;
	}

	std::erase_if(quadArbiters, [](const auto& pair) { return pair.second.stale; });
	std::swap(prevQuads, currQuads);
}

void RegionArbiter::updateArbiters(secs deltaTime) {
	for (auto& [quad, arbiter] : quadArbiters) {
		arbiter.update(deltaTime);
	}
}

}