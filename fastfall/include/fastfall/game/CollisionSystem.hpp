#pragma once

#include "fastfall/game/phys/ColliderRegion.hpp"
#include "fastfall/game/phys/CollidableArbiter.hpp"
#include "fastfall/game/phys/RegionArbiter.hpp"
#include "fastfall/game/phys/collision/Contact.hpp"
#include "fastfall/game/WorldState.hpp"

#include "ext/plf_colony.h"
#include "nlohmann/json_fwd.hpp"

#include <vector>
#include <list>
#include <memory>
#include "fastfall/util/slot_map.hpp"

namespace ff {

class CollisionSystem : public WorldStateListener {
public:

	CollisionSystem(WorldState& st);
	void update(WorldState& st, secs deltaTime);

	inline void dumpCollisionDataThisFrame(nlohmann::ordered_json* dump_ptr) { collision_dump = dump_ptr; };
	inline void resetFrameCount() { frame_count = 0; };
	inline size_t getFrameCount() const { return frame_count; };

	void notify_component_created(WorldState& st, ID<Entity> entity, GenericID gid) override;
	void notify_component_destroyed(WorldState& st, ID<Entity> entity, GenericID gid) override;

private:
	size_t frame_count = 0;
	size_t frame_collision_count = 0;

	nlohmann::ordered_json* collision_dump = nullptr;

	std::unordered_map<ID<Collidable>, std::vector<RegionArbiter>> collisions;

	void gather_collisions(
		WorldState& st,
		secs deltaTime,
		ID<Collidable> collidable_id,
		nlohmann::ordered_json* dump_ptr = nullptr);

	void solve_collisions(
		WorldState& st,
		secs deltaTime,
		ID<Collidable> collidable_id, 
		nlohmann::ordered_json* dump_ptr = nullptr);

	void update_region_arbiters(WorldState& st, ID<Collidable> collidable_id, Rectf bounds);
	Rectf push_bounds_for_contact(Rectf push_bound, const cardinal_array<float>& boundDist, const Contact* contact);
};

}
