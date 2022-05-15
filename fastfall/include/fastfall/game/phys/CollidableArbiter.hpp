#pragma once

#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/RegionArbiter.hpp"

#include "nlohmann/json_fwd.hpp"

namespace ff {

class CollidableArbiter {
public:
	Collidable collidable;
	std::vector<RegionArbiter> region_arbiters;

	inline void gather_and_solve_collisions(
			secs deltaTime, 
			const std::vector<std::unique_ptr<ColliderRegion>>& regions,
			nlohmann::ordered_json* dump_ptr = nullptr) 
	{
		gather_collisions(deltaTime, regions, dump_ptr);
		solve_collisions(dump_ptr);
	};
	void erase_region(const ColliderRegion* region);

	size_t frame_count = 0;
private:

	void gather_collisions(secs deltaTime, const std::vector<std::unique_ptr<ColliderRegion>>& regions, nlohmann::ordered_json* dump_ptr = nullptr);
	void solve_collisions(nlohmann::ordered_json* dump_ptr = nullptr);

	void update_region_arbiters(Rectf bounds, const std::vector<std::unique_ptr<ColliderRegion>>& regions);
	Rectf push_bounds_for_contact(Rectf push_bound, const cardinal_array<float>& boundDist, const Contact* contact);
};

}

