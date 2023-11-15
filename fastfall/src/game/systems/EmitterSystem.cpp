#include "fastfall/game/systems/EmitterSystem.hpp"

#include "fastfall/game/World.hpp"

namespace ff {


void EmitterSystem::update(World& world, secs deltaTime) {
    if (deltaTime > 0.0) {

        events.clear();
        events_per_emitter.clear();

        const poly_id_map<ColliderRegion>& collider_regions = world.all<ColliderRegion>();
        for (auto [eid, e]: world.all<Emitter>()) {
            size_t init_events_count = events.size();
            auto output_it = std::back_inserter(events);
            e.update(deltaTime, &output_it);
            e.apply_collision(collider_regions, &output_it);
            events_per_emitter.push_back(events.size() - init_events_count);
        }

        auto count_it  = events_per_emitter.begin();
        auto events_it = events.begin();
        for (auto [eid, e]: world.all<Emitter>()) {
            auto count = *count_it;
            auto event_span = std::span{events_it, events_it + count};

            if (!event_span.empty() && e.strategy.events_callback) {
                e.strategy.events_callback(world, event_span);
            }

            events_it += count;
            ++count_it;
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
