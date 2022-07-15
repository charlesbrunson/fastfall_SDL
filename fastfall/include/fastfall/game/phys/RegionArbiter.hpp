#pragma once

#include "fastfall/game/phys/ColliderRegion.hpp"

#include "fastfall/game/phys/Arbiter.hpp"
#include "fastfall/game/ID.hpp"

#include <map>

namespace ff {

class RegionArbiter {
public:
	void updateRegion(Rectf bounds);

	bool operator< (const RegionArbiter& rhs) {
		return collider_id < rhs.collider_id;
	}

	ID<ColliderRegion> collider_id;
	std::vector<std::pair<Rectf, QuadID>> currQuads;
	std::map<QuadID, Arbiter> quadArbiters;
};

}

