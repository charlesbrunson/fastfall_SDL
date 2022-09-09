#pragma once

#include "fastfall/game_v2/phys/ColliderRegion.hpp"
#include "fastfall/game_v2/phys/CollidableArbiter.hpp"
#include "fastfall/game_v2/phys/RegionArbiter.hpp"
#include "fastfall/game_v2/phys/collision/Contact.hpp"

#include "ext/plf_colony.h"
#include "nlohmann/json_fwd.hpp"

#include <vector>
#include <list>
#include <memory>

namespace ff {

class World;

class CollisionSystem {
public:
	void update(secs deltaTime);

    inline void set_world(World* w) { world = w; }

    void notify_created(ID<Collidable> id);
    void notify_created(ID<ColliderRegion> id);

    void notify_erased(ID<Collidable> id);
    void notify_erased(ID<ColliderRegion> id);

	// dump collision data from this frame into json, is reset at the end of the update
	inline void dumpCollisionDataThisFrame(nlohmann::ordered_json* dump_ptr) { collision_dump = dump_ptr; };

	inline void resetFrameCount() { frame_count = 0; };
	inline size_t getFrameCount() const { return frame_count; };

    Arbiter* get_arbiter(Contact contact);

private:
    std::unordered_map<ID<Collidable>, CollidableArbiter> arbiters;

	size_t frame_count = 0;
	size_t frame_collision_count = 0;
	
	nlohmann::ordered_json* collision_dump = nullptr;
    World* world;

};

}
