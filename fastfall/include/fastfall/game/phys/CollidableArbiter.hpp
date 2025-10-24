#pragma once

#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/RegionArbiter.hpp"
#include "fastfall/util/id.hpp"

#include "nlohmann/json_fwd.hpp"
#include "fastfall/util/id_map.hpp"

#include <span>
#include <unordered_map>

namespace ff {

class World;

class CollidableArbiter {
public:
    ID<Collidable> collidable_id;
	std::unordered_map<ID<ColliderRegion>, RegionArbiter> region_arbiters;

	void gather_and_solve_collisions(
            World& world,
			secs deltaTime,
			nlohmann::ordered_json* dump_ptr = nullptr)
	{
		gather_collisions(world, deltaTime, dump_ptr);
		solve_collisions(world, deltaTime, dump_ptr);
	}
	void erase_region(ID<ColliderRegion> region);

    [[nodiscard]] Arbiter* get_quad_arbiter(CollisionID id);

private:
	void gather_collisions(
            World& world,
            secs deltaTime,
            nlohmann::ordered_json* dump_ptr = nullptr);

	void solve_collisions(
            World& world,
            secs deltaTime,
            nlohmann::ordered_json* dump_ptr = nullptr);

	void update_region_arbiters(
            World& world,
            Rectf bounds);

	Rectf push_bounds_for_contact(
            Rectf init_push_bound,
            const cardinal_array<float>& boundDist,
            const ContinuousContact& contact);

};

}

