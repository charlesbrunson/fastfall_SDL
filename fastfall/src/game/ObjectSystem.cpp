#include "fastfall/game/ObjectSystem.hpp"

#include "fastfall/game/World.hpp"

namespace ff {

void ObjectSystem::update(World& world, secs deltaTime)
{
    append_created();
    for (auto obj : update_order) {
        world.at(obj).update(world, deltaTime);
    }
    append_created();
}

void ObjectSystem::predraw(World& world, float interp, bool updated)
{
    for (auto& id : update_order) {
        auto& obj = world.at(id);
        if (obj.should_delete()) {
            world.erase(id);
        }
    }
}

void ObjectSystem::notify_created(World& world, ID<GameObject> id) {
    created_objects.push_back(id);
}

void ObjectSystem::notify_erased(World& world, ID<GameObject> id) {
    //world.at(id).clean(world);
    std::erase(update_order, id);
    std::erase(created_objects, id);
}

}
