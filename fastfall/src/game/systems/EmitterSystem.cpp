#include "fastfall/game/systems/EmitterSystem.hpp"

#include "fastfall/game/World.hpp"

namespace ff {


void EmitterSystem::update(World& world, secs deltaTime) {
    if (deltaTime > 0.0) {
        for (auto [eid, e]: world.all<Emitter>()) {
            e.update(deltaTime);
        }
    }
}

void EmitterSystem::predraw(World& world, float interp, bool updated) {
    for (auto [eid, e] : world.all<Emitter>())
    {
        e.predraw(
                world.at(e.get_drawid()),
                world.system<SceneSystem>().config(e.get_drawid()),
                interp,
                updated);
    }
}

void EmitterSystem::notify_created(World &world, ID<Emitter> id) {
    auto varr_id = world.create<VertexArray>(
            world.entity_of(id),
            ff::Primitive::TRIANGLES);

    world.at(id).set_drawid(varr_id);
}

void EmitterSystem::notify_erased(World &world, ID<Emitter> id) {
    world.erase(world.at(id).get_drawid());
}

}
