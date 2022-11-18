#pragma once

#include "fastfall/game/phys/ColliderRegion.hpp"
#include "fastfall/game/phys/CollidableArbiter.hpp"
#include "fastfall/game/phys/RegionArbiter.hpp"
#include "fastfall/game/phys/collision/Contact.hpp"

#include "ext/plf_colony.h"
#include "nlohmann/json_fwd.hpp"

#include <vector>
#include <list>
#include <memory>
#include "fastfall/util/slot_map.hpp"

namespace ff {

class CollisionSystem {
public:
	using collidables_vector = std::vector<std::unique_ptr<CollidableArbiter>>;
	using regions_vector = std::vector<std::unique_ptr<ColliderRegion>>;

private:
	collidables_vector collidables;
	regions_vector regions;

	//slot_map<Collidable> collidables;
	//slot_map<std::unique_ptr<ColliderRegion>> colliders;
	//std::vector<CollidableArbiter> arbiters;

public:

	CollisionSystem();

	void update(secs deltaTime);
	
	Collidable* create_collidable(Vec2f init_pos, Vec2f init_size, Vec2f init_grav = Vec2f{});
	bool erase_collidable(Collidable* collidable);
	
	template<ColliderType T, typename ... Args>
	T* create_collider(Args&&... args) {
		std::unique_ptr<ColliderRegion> collider = std::make_unique<T>(std::forward<Args>(args)...);
		T* collider_ptr = static_cast<T*>(collider.get());
		regions.push_back(std::move(collider));
		return collider_ptr;
	}
	bool erase_collider(ColliderRegion* region);


	inline const regions_vector& 		get_colliders() 	const { return regions; };
	inline const collidables_vector& 	get_collidables() 	const { return collidables; };

	// dump collision data from this frame into json, is reset at the end of the update
	inline void dumpCollisionDataThisFrame(nlohmann::ordered_json* dump_ptr) { collision_dump = dump_ptr; };

	inline void resetFrameCount() { frame_count = 0; };
	inline size_t getFrameCount() const { return frame_count; };

private:

	size_t frame_count = 0;
	size_t frame_collision_count = 0;
	
	nlohmann::ordered_json* collision_dump = nullptr;

};

}
