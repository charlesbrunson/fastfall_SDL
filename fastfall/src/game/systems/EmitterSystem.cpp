#include "fastfall/game/systems/EmitterSystem.hpp"

#include "fastfall/game/World.hpp"

namespace ff {


void EmitterSystem::update(World& world, secs deltaTime) {
    if (deltaTime > 0.0) {
        const poly_id_map<ColliderRegion>& collider_regions = world.all<ColliderRegion>();
        for (auto [eid, e]: world.all<Emitter>()) {
            e.update(deltaTime);
            e.apply_collision(collider_regions);
        }
    }
}

void EmitterSystem::predraw(World& world, predraw_state_t predraw_state) {
    for (auto [eid, e] : world.all<Emitter>())
    {
        e.predraw(
                world.at(e.get_drawid()),
                world.system<SceneSystem>().config(e.get_drawid()),
                predraw_state);
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
