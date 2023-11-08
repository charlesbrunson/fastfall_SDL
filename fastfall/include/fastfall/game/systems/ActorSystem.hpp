#pragma once

#include "fastfall/game/actor/Actor.hpp"

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/id.hpp"

#include <list>
#include <map>


namespace ff {

class World;

class ActorSystem {
public:
	void update(World& world, secs deltaTime);
	void predraw(World& world, predraw_state_t predraw_state);

    void notify_created(World& world, ID<Actor> id);
    void notify_erased(World& world, ID<Actor> id);

protected:
    void append_created(const World& world);

    std::vector<ID<Actor>> update_order;
    std::vector<ID<Actor>> created_actors;
};

}
