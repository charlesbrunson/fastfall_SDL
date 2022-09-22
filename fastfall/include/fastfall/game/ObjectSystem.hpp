#pragma once

#include "fastfall/game/object/GameObject.hpp"

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/id.hpp"

#include <list>
#include <map>


namespace ff {

class World;

class ObjectSystem {
public:
	void update(World& world, secs deltaTime);
	void predraw(World& world, float interp, bool updated);

    void notify_created(World& world, ID<GameObject> id);
    void notify_erased(World& world, ID<GameObject> id);
};

}
