#pragma once

#include "fastfall/game_v2/phys/ColliderRegion.hpp"
#include "fastfall/game_v2/phys/Arbiter.hpp"
#include "fastfall/util/id.hpp"

#include <map>

namespace ff {

class RegionArbiter {
public:
	RegionArbiter(
            ID<ColliderRegion> collider_id_,
            ID<Collidable> collidable_id_
        )
        : collider_id(collider_id_)
        , collidable_id(collidable_id_)
	{
	}

	inline auto& getQuadArbiters() { return quadArbiters; };
	inline const auto& getQuadArbiters() const { return quadArbiters; };

	void updateRegion(CollisionContext ctx, Rectf bounds);

    ID<Collidable> get_collidable_id() const { return collidable_id; }
    ID<ColliderRegion> get_collider_id() const { return collider_id; }

	bool operator< (const RegionArbiter& rhs) {
		return collider_id < rhs.collider_id;
	}

private:
	ID<ColliderRegion> collider_id;
	ID<Collidable> collidable_id;

	std::vector<std::pair<Rectf, QuadID>> currQuads;
	std::map<QuadID, Arbiter> quadArbiters;

};

}

