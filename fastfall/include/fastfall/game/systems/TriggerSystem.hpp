#pragma once

#include "fastfall/game/trigger/Trigger.hpp"
#include "fastfall/util/slot_map.hpp"

#include <set>

namespace ff {

class World;

class TriggerSystem {
public:
    void update(World& world, secs deltaTime);
    void notify_created(World& world, ID<Trigger> id);
    void notify_erased(World& world, ID<Trigger> id);

private:
	void compareTriggers(World& w, Trigger& A, Trigger& B, secs deltaTime);
};

}
