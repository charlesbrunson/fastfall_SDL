#include "fastfall/game/ObjectSystem.hpp"

#include "fastfall/game/World.hpp"

namespace ff {

void ObjectSystem::update(World& world, secs deltaTime) {
    for (auto& obj : world.all<GameObject>()) {
        obj->update(world, deltaTime);
    }
}

void ObjectSystem::predraw(World& world, float interp, bool updated)
{
    auto& objects = world.all<GameObject>();
    for (auto& obj : objects) {
        if (obj->should_delete()) {
            world.erase(objects.id_of(obj));
        }
        else {
            obj->predraw(world, interp, updated);
        }
    }
}

void ObjectSystem::notify_created(World& world, ID<GameObject> id) {

}

void ObjectSystem::notify_erased(World& world, ID<GameObject> id) {
    world.at(id).clean(world);
}

}
