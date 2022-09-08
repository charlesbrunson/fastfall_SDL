#pragma once

#include "fastfall/game_v2/phys/Collidable.hpp"
#include "fastfall/game_v2/phys/RegionArbiter.hpp"
#include "fastfall/util/id.hpp"

#include "nlohmann/json_fwd.hpp"

namespace ff {

class World;

class CollidableArbiter {
public:
    World* world;
    ID<Collidable> collidable;
	std::vector<RegionArbiter> region_arbiters;

	inline void gather_and_solve_collisions(
			secs deltaTime, 
			const std::vector<std::unique_ptr<ColliderRegion>>& regions,
			nlohmann::ordered_json* dump_ptr = nullptr) 
	{
		gather_collisions(deltaTime, regions, dump_ptr);
		solve_collisions(dump_ptr);
	};
	void erase_region(ID<ColliderRegion> region);
	
private:

	void gather_collisions(secs deltaTime, const std::vector<std::unique_ptr<ColliderRegion>>& regions, nlohmann::ordered_json* dump_ptr = nullptr);
	void solve_collisions(nlohmann::ordered_json* dump_ptr = nullptr);

	void update_region_arbiters(Rectf bounds, const std::vector<std::unique_ptr<ColliderRegion>>& regions);
	Rectf push_bounds_for_contact(Rectf push_bound, const cardinal_array<float>& boundDist, const Contact* contact);
};

}

