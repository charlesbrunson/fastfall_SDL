#pragma once

#include "fastfall/game/Actor.hpp"

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/id.hpp"

#include <list>
#include <map>


namespace ff {

class World;

class ActorSystem {
public:
	void update(World& world, secs deltaTime);
	void predraw(World& world, float interp, bool updated);

    void notify_created(World& world, ID<Actor> id);
    void notify_erased(World& world, ID<Actor> id);

protected:
    void append_created() {
        if (!created_actors.empty()) {
            update_order.insert(update_order.end(),
                                created_actors.begin(), created_actors.end());
            created_actors.clear();
        }
    }

    std::vector<ID<Actor>> update_order;
    std::vector<ID<Actor>> created_actors;
};

}
