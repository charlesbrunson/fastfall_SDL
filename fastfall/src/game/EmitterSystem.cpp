#include "fastfall/game/EmitterSystem.hpp"

#include "fastfall/game/World.hpp"

namespace ff {


void EmitterSystem::update(World& world, secs deltaTime) {
    for (auto [eid, e] : world.all<Emitter>())
    {
        e.update(deltaTime);
    }
}

void EmitterSystem::predraw(World& world, float interp, bool updated) {
    for (auto [eid, e] : world.all<Emitter>())
    {
        e.predraw(
                world.at(e.get_vertexarray()),
                world.scene().config(e.get_vertexarray()),
                interp,
                updated);
    }
}

void EmitterSystem::notify_created(World &world, ID<Emitter> id) {
    auto varr_id = world.create_drawable<VertexArray>(
            world.get_entity_of(id),
            ff::Primitive::TRIANGLES);

    world.at(id).set_vertexarray(varr_id);
}

void EmitterSystem::notify_erased(World &world, ID<Emitter> id) {

}

}
