#include "fastfall/game/EmitterSystem.hpp"

#include "fastfall/game/World.hpp"

namespace ff {


void EmitterSystem::update(World& world, secs deltaTime) {
    for (auto [eid, e] : world.all<Emitter>())
    {
        e.update(world, deltaTime);
    }
}

void EmitterSystem::predraw(World& world, float interp, bool updated) {
    for (auto [eid, e] : world.all<Emitter>())
    {
        e.predraw(world, interp, updated);

    }
}

void EmitterSystem::notify_created(World &world, ID<Emitter> id) {
    auto varr_id = world.create_drawable<VertexArray>(ff::Primitive::TRIANGLES);
    world.at(id).set_vertexarray(varr_id);
}

void EmitterSystem::notify_erased(World &world, ID<Emitter> id) {
    world.erase(world.at(id).get_vertexarray());
}

}
