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
        //active->update(*this, deltaTime);
        state._level_system.update(*this, deltaTime);
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
        //active->predraw(*this, interp, updated);
        state._level_system.predraw(*this, interp, updated);
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


ID<Entity> World::create_entity() {
    return state._entities.create();
}

std::optional<ID<GameObject>> World::create_object_from_data(ObjectLevelData& data) {
    auto ent_id = create_entity();
    auto obj_id = state._objects.peek_next_id();
    tie_component_entity(obj_id, ent_id);
    auto ptr = ObjectFactory::createFromData(*this, obj_id, data);
    if (ptr) {
        state._objects.emplace(std::move(ptr));
        system_notify_created(obj_id);
        return obj_id;
    }
    else {
        return {};
    }
}

ID<Level> World::create_level(const LevelAsset& levelData, bool create_objects) {
    auto id = create_entity();
    auto lvl_id = create<Level>(id, *this, id_placeholder, levelData);
    system_notify_created(lvl_id);
    if (create_objects) {
        at(lvl_id).get_layers().get_obj_layer().createObjectsFromData(*this);
    }
    return lvl_id;
}

ID<Level> World::create_level(std::optional<std::string> name, std::optional<Vec2u> size, std::optional<Color> bg_color) {
    auto id = create_entity();
    auto lvl_id = create<Level>(id, *this, id_placeholder, name, size, bg_color);
    system_notify_created(lvl_id);
    auto& lvl = at(lvl_id);
    return lvl_id;
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
    auto ent = entity_of(component);
    std::visit([&, this]<typename T>(ID<T> id) {
            system_notify_erased(id);
            untie_component_entity(id, ent);
            list_for<T>().erase(id);
        }, component);
    return true;
}

const std::set<ComponentID>& World::components_of(ID<Entity> id) const {
    return state._entities.at(id).components;
}

ID<Entity> World::entity_of(ComponentID id) const {
    return state._comp_to_ent.at(id);
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