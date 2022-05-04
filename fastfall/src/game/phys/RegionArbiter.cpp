#include "fastfall/game/phys/RegionArbiter.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/render/DebugDraw.hpp"

namespace ff {

void RegionArbiter::updateRegion(Rectf bounds) {

	currQuads.clear();
	collider_->getQuads(bounds, currQuads);

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
			iter = quadArbiters.insert(iter, std::make_pair(q.second, Arbiter(collidable_, q.second, collider_)));
		}
		iter->second.stale = false;
	}

	std::erase_if(quadArbiters, [](const auto& pair) { return pair.second.stale; });
	//std::swap(prevQuads, currQuads);
	
	/*
	if (debug_draw::hasTypeEnabled(debug_draw::Type::COLLISION_CONTACT)) {
		for (auto& arb : quadArbiters)
		{
			if (!debug_draw::repeat(&arb, Vec2f{}))
			{
				Rectf bound;
				auto& shape = createDebugDrawable<VertexArray, debug_draw::Type::COLLISION_CONTACT>((const void*)&arb, Primitive::LINES, 8);
				size_t n = 0;
				for (auto& side : arb.first->surfaces)
				{
					if (side.hasSurface) {
						shape[n].pos		= side.collider.surface.p1;
						shape[n + 1].pos	= side.collider.surface.p2;

						shape[n].color		= Color::Red;
						shape[n + 1].color	= Color::Red;
					}
					n += 2;
				}
			}
		}
	}
	*/
}

}