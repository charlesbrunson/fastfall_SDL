#pragma once

#include "fastfall/game_v2/phys/ColliderRegion.hpp"
#include "fastfall/game_v2/phys/Arbiter.hpp"
#include "fastfall/util/id.hpp"

#include <map>

namespace ff {

class RegionArbiter {
public:
	RegionArbiter(ID<ColliderRegion> collider, ID<Collidable> collidable) :
		collider_(collider),
		collidable_(collidable)
	{

	}

	inline std::map<const ColliderQuad*, Arbiter>& getQuadArbiters() { return quadArbiters; };
	inline const std::map<const ColliderQuad*, Arbiter>& getQuadArbiters() const { return quadArbiters; };

	void updateRegion(Rectf bounds);

	ID<ColliderRegion> getRegion() const { return collider_; };

	bool operator< (const RegionArbiter& rhs) {
		return collider_ < rhs.collider_;
	}

private:
	ID<ColliderRegion> collider_;
	ID<Collidable> collidable_;

	std::vector<std::pair<Rectf, const ColliderQuad*>> currQuads;
	std::map<const ColliderQuad*, Arbiter> quadArbiters;

};

}

