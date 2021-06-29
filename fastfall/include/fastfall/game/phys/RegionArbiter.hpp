#pragma once

#include "fastfall/game/phys/ColliderRegion.hpp"

#include "fastfall/game/phys/Arbiter.hpp"

#include <map>

namespace ff {

class RegionArbiter {
public:
	RegionArbiter(ColliderRegion* collider, Collidable* collidable) :
		collider_(collider),
		collidable_(collidable)
	{

	}

	inline std::map<const ColliderQuad*, Arbiter>& getQuadArbiters() { return quadArbiters; };

	void updateRegion(Rectf bounds);
	void updateArbiters(secs deltaTime);

	ColliderRegion* getRegion() const { return collider_; };

	bool operator< (const RegionArbiter& rhs) {
		return collider_ < rhs.collider_;
	}

private:
	ColliderRegion* collider_;
	Collidable* collidable_;

	std::vector<std::pair<Rectf, const ColliderQuad*>> prevQuads;
	std::vector<std::pair<Rectf, const ColliderQuad*>> currQuads;
	std::map<const ColliderQuad*, Arbiter> quadArbiters;

};

}

