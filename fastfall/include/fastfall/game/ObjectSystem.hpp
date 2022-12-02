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

protected:
    void append_created() {
        if (!created_objects.empty()) {
            update_order.insert(update_order.end(),
                                created_objects.begin(), created_objects.end());
            created_objects.clear();
        }
    }
    std::vector<ID<GameObject>> update_order;
    std::vector<ID<GameObject>> created_objects;
};

}
