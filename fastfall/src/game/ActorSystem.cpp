#include "fastfall/game/ActorSystem.hpp"

#include "fastfall/game/World.hpp"

namespace ff {

void ActorSystem::update(World& world, secs deltaTime)
{
    append_created();
    for (auto obj : update_order) {
        world.at(obj).update(world, deltaTime);
    }
    append_created();
}

void ActorSystem::predraw(World& world, float interp, bool updated)
{
    std::vector<ID<Entity>> to_erase;
    for (auto& id : update_order) {
        auto& actor = world.at(id);
        if (actor.is_dead()) {
            to_erase.push_back(actor.entity_id);
        }
    }
    for (auto ent : to_erase) {
        world.erase(ent);
    }
}

void ActorSystem::notify_created(World& world, ID<Actor> id) {
    created_actors.push_back(id);
}

void ActorSystem::notify_erased(World& world, ID<Actor> id) {
    std::erase(update_order, id);
    std::erase(created_actors, id);
}

}
