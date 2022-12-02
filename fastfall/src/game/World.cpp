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
}

World::World(World&& other) noexcept
{
    WorldImGui::add(this);
    state = std::move(other.state);
}

World& World::operator=(const World& other) {
    WorldImGui::add(this);
    state = other.state;
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
        active->update(*this, deltaTime);
        state._trigger_system.update(*this, deltaTime);

        state._object_system.update(*this, deltaTime);
        state._path_system.update(*this, deltaTime);
        state._attach_system.update_attachpoints(*this, deltaTime, AttachPoint::Schedule::PostUpdate);

        state._collision_system.update(*this, deltaTime);

        state._attach_system.update_attachpoints(*this, deltaTime, AttachPoint::Schedule::PostCollision);
        state._camera_system.update(*this, deltaTime);
        state._emitter_system.update(*this, deltaTime);

        if (deltaTime > 0.0)
            state.update_counter++;

        state.update_time += deltaTime;
    }
}

void World::predraw(float interp, bool updated) {
    if (Level* active = state._level_system.get_active(*this))
    {
        state._scene_system.set_bg_color(active->getBGColor());
        state._scene_system.set_size(active->size());
        state._object_system.predraw(*this, interp, updated);
        active->predraw(*this, interp, updated);
        state._emitter_system.predraw(*this, interp, updated);
        state._scene_system.set_cam_pos(state._camera_system.getPosition(interp));
        state._scene_system.predraw(*this, interp, updated);
    }
    else
    {
        state._scene_system.set_bg_color(ff::Color::Transparent);
    }
}

void World::draw(RenderTarget& target, RenderState t_state) const
{
    state._scene_system.draw(*this, target, t_state);
}

bool World::erase(ID<Entity> entity) {
    auto components = state._entities.at(entity).components;
    for (auto& c : components) {
        erase(c);
    }
    state._entities.erase(entity);
    return true;
}
bool World::erase(ComponentID component) {
    auto ent = state._comp_to_ent.at(component);
    std::visit([this]<typename T>(ID<T> id) {
            system_notify_erased(id);
        }, component);
    state._entities.at(ent).components.erase(component);
    return true;
}

const std::set<ComponentID>& World::get_components_of(ID<Entity> id) const {
    return state._entities.at(id).components;
}

ID<Entity> World::get_entity_of(ComponentID id) const {
    return state._comp_to_ent.at(id);
}

}