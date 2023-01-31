#include "fastfall/game/World.hpp"

#include "fastfall/game/WorldImGui.hpp"

#include "fastfall/render/DebugDraw.hpp"

namespace ff {

World::World()
{
    WorldImGui::add(this);
}

World::World(const World& other)
{
    WorldImGui::add(this);
    state = other.state;
    state._scene_system.reset_proxy_ptrs(state._drawables);
}

World::World(World&& other) noexcept
{
    WorldImGui::add(this);
    state = std::move(other.state);
}

World& World::operator=(const World& other) {
    WorldImGui::add(this);
    state = other.state;
    state._scene_system.reset_proxy_ptrs(state._drawables);
    return *this;
}

World& World::operator=(World&& other) noexcept {
    WorldImGui::add(this);
    state = std::move(other.state);
    return *this;
}

World::~World() {
    WorldImGui::remove(this);
}

void World::update(secs deltaTime) {
    if (Level* active = state._level_system.get_active(*this))
    {
        state._scene_system.update(*this, deltaTime);
        state._attach_system.update(*this, deltaTime);
        state._input.update(deltaTime);
        state._level_system.update(*this, deltaTime);

        // TODO remake into actor system
        state._object_system.update(*this, deltaTime);

        state._trigger_system.update(*this, deltaTime);
        state._path_system.update(*this, deltaTime);
        state._attach_system.update_attachpoints(*this, deltaTime, AttachPoint::Schedule::PostUpdate);

        state._collision_system.update(*this, deltaTime);
        state._attach_system.update_attachpoints(*this, deltaTime, AttachPoint::Schedule::PostCollision);

        state._camera_system.update(*this, deltaTime);
        state._emitter_system.update(*this, deltaTime);

        if (deltaTime > 0.0) {
            state.update_counter++;
        }
        state.update_time += deltaTime;
    }
}

void World::predraw(float interp, bool updated)
{
    if (Level* active = state._level_system.get_active(*this))
    {
        /*
        if (updated && active->try_reload_level(*this)) {
            for (auto [id, actor] : all<Actor>()) {
                actor->notify_active_level_reloaded(*this);
            }
        }
        */

        state._scene_system.set_bg_color(active->getBGColor());
        state._scene_system.set_size(active->size());
        state._object_system.predraw(*this, interp, updated);
        state._level_system.predraw(*this, interp, updated);
        state._emitter_system.predraw(*this, interp, updated);
        state._scene_system.set_cam_pos(state._camera_system.getPosition(interp));
        state._scene_system.predraw(*this, interp, updated);
    }
    else
    {
        state._scene_system.set_bg_color(ff::Color::Transparent);
    }
    clean_drawables();
}

void World::draw(RenderTarget& target, RenderState t_state) const
{
    state._scene_system.draw(*this, target, t_state);
}

bool World::erase(ID<Entity> entity) {
    auto& ent = state._entities.at(entity);
    auto actor = ent.actor;
    auto components = ent.components;
    if (actor) {
        state._actors.erase(*actor);
    }
    for (auto& c : components) {
        erase(c);
    }
    state._entities.erase(entity);
    return true;
}
bool World::erase(ComponentID component) {
    auto ent = entity_of(component);
    std::visit([&, this]<typename T>(ID<T> id) {
            system_notify_erased(id);
            untie_component_entity(id, ent);
            if constexpr (std::same_as<T, Drawable>) {
                state.erase_drawables_deferred.insert(
                    std::lower_bound(
                            state.erase_drawables_deferred.begin(),
                            state.erase_drawables_deferred.end(),
                            id),
                    id);
            }
            else {
                list_for<T>().erase(id);
            }
        }, component);
    return true;
}
void World::clean_drawables() {
   for(auto id : state.erase_drawables_deferred) {
       list_for<Drawable>().erase(id);
   }
   state.erase_drawables_deferred.clear();
}
bool World::due_to_erase(ID<Drawable> id) const {
   return std::binary_search(state.erase_drawables_deferred.begin(), state.erase_drawables_deferred.end(), id);
}

const std::set<ComponentID>& World::components_of(ID<Entity> id) const {
    return state._entities.at(id).components;
}

ID<Entity> World::entity_of(ComponentID id) const {
    return state._comp_to_ent.at(id);
}

ID<Entity> World::entity_of(ID<Actor> id) const {
    return state._actors.at(id).entity_id;
}

bool World::entity_has_actor(ID<Entity> id) const {
    return state._entities.at(id).actor.has_value();
}

void World::tie_component_entity(ComponentID cmp, ID<Entity> ent) {
    state._entities.at(ent).components.emplace(cmp);
    state._comp_to_ent.emplace(cmp, ent);
}

void World::untie_component_entity(ComponentID cmp, ID<Entity> ent) {
    state._entities.at(ent).components.erase(cmp);
    state._comp_to_ent.erase(cmp);
}

}