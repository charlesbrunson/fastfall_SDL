#pragma once

#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/RegionArbiter.hpp"
#include "fastfall/util/id.hpp"

#include "nlohmann/json_fwd.hpp"
#include "fastfall/util/id_map.hpp"

#include <span>

namespace ff {

class World;

class CollidableArbiter {
public:
    ID<Collidable> collidable_id;
	std::unordered_map<ID<ColliderRegion>, RegionArbiter> region_arbiters;

	inline void gather_and_solve_collisions(
            Collidable& collidable,
            poly_id_map<ColliderRegion>& colliders,
			secs deltaTime,
			nlohmann::ordered_json* dump_ptr = nullptr)
	{
		gather_collisions(collidable, colliders, deltaTime, dump_ptr);
		solve_collisions(collidable, colliders, dump_ptr);
	};
	void erase_region(ID<ColliderRegion> region);

    Arbiter* get_quad_arbiter(CollisionID id);

private:
	void gather_collisions(
            Collidable& collidable,
            poly_id_map<ColliderRegion>& colliders,
            secs deltaTime,
            nlohmann::ordered_json* dump_ptr = nullptr);

	void solve_collisions(
            Collidable& collidable,
            poly_id_map<ColliderRegion>& colliders,
            nlohmann::ordered_json* dump_ptr = nullptr);

	void update_region_arbiters(
            Collidable& collidable,
            poly_id_map<ColliderRegion>& colliders,
            Rectf bounds);

	Rectf push_bounds_for_contact(
            Rectf init_push_bound,
            const cardinal_array<float>& boundDist,
            const ContinuousContact& contact);

};

}

