#pragma once

#include "fastfall/game_v2/trigger/Trigger.hpp"
#include "fastfall/util/slot_map.hpp"

#include <set>

namespace ff {

class World;

class TriggerSystem {
public:
    void update(secs deltaTime);

    inline void set_world(World* w) { world = w; }
    void notify_created(ID<Trigger> id);
    void notify_erased(ID<Trigger> id);

private:
	void compareTriggers(Trigger& A, Trigger& B, secs deltaTime);
    World* world = nullptr;
};

}
