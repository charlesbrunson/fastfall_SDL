#include "fastfall/game/World.hpp"

#include "fastfall/game/WorldImGui.hpp"

namespace ff {

World::World()
{
    WorldImGui::add(this);
}

World::World(const World& other)
{
    WorldImGui::add(this);
    _objects = other._objects;
    _levels = other._levels;
    _collidables = other._collidables;
    _colliders = other._colliders;
    _triggers = other._triggers;
    _emitters = other._emitters;
    _input = other._input;
    _camera_targets = other._camera_targets;
    _drawables = other._drawables;
    _attachpoints = other._attachpoints;
    _level_system = other._level_system;
    _object_system = other._object_system;
    _collision_system = other._collision_system;
    _trigger_system = other._trigger_system;
    _camera_system = other._camera_system;
    _scene_system = other._scene_system;
    _emitter_system = other._emitter_system;
    _attach_system = other._attach_system;
    ent_to_comp = other.ent_to_comp;
    comp_to_ent = other.comp_to_ent;
    update_counter = other.update_counter;
    update_time = other.update_time;
}

World::World(World&& other) noexcept
{
    WorldImGui::add(this);
    _objects = std::move(other._objects);
    _levels = std::move(other._levels);
    _collidables = std::move(other._collidables);
    _colliders = std::move(other._colliders);
    _triggers = std::move(other._triggers);
    _emitters = std::move(other._emitters);
    _input = std::move(other._input);
    _camera_targets = std::move(other._camera_targets);
    _drawables = std::move(other._drawables);
    _attachpoints = std::move(other._attachpoints);
    _level_system = std::move(other._level_system);
    _object_system = std::move(other._object_system);
    _collision_system = std::move(other._collision_system);
    _trigger_system = std::move(other._trigger_system);
    _camera_system = std::move(other._camera_system);
    _scene_system = std::move(other._scene_system);
    _emitter_system = std::move(other._emitter_system);
    _attach_system = std::move(other._attach_system);
    ent_to_comp = std::move(other.ent_to_comp);
    comp_to_ent = std::move(other.comp_to_ent);
    update_counter = other.update_counter;
    update_time = other.update_time;
}

World& World::operator=(const World& other) {
    WorldImGui::add(this);
    _objects = other._objects;
    _levels = other._levels;
    _collidables = other._collidables;
    _colliders = other._colliders;
    _triggers = other._triggers;
    _emitters = other._emitters;
    _input = other._input;
    _camera_targets = other._camera_targets;
    _drawables = other._drawables;
    _attachpoints = other._attachpoints;
    _level_system = other._level_system;
    _object_system = other._object_system;
    _collision_system = other._collision_system;
    _trigger_system = other._trigger_system;
    _camera_system = other._camera_system;
    _scene_system = other._scene_system;
    _emitter_system = other._emitter_system;
    _attach_system = other._attach_system;
    ent_to_comp = other.ent_to_comp;
    comp_to_ent = other.comp_to_ent;
    update_counter = other.update_counter;
    update_time = other.update_time;
    return *this;
}

World& World::operator=(World&& other) noexcept {
    WorldImGui::add(this);
    _objects = std::move(other._objects);
    _levels = std::move(other._levels);
    _collidables = std::move(other._collidables);
    _colliders = std::move(other._colliders);
    _triggers = std::move(other._triggers);
    _emitters = std::move(other._emitters);
    _input = std::move(other._input);
    _camera_targets = std::move(other._camera_targets);
    _drawables = std::move(other._drawables);
    _attachpoints = std::move(other._attachpoints);
    _level_system = std::move(other._level_system);
    _object_system = std::move(other._object_system);
    _collision_system = std::move(other._collision_system);
    _trigger_system = std::move(other._trigger_system);
    _camera_system = std::move(other._camera_system);
    _scene_system = std::move(other._scene_system);
    _emitter_system = std::move(other._emitter_system);
    _attach_system = std::move(other._attach_system);
    ent_to_comp = std::move(other.ent_to_comp);
    comp_to_ent = std::move(other.comp_to_ent);
    update_counter = other.update_counter;
    update_time = other.update_time;
    return *this;
}

World::~World() {
    WorldImGui::remove(this);
}

ID<Collidable> World::create_collidable(EntityID ent, Vec2f position, Vec2f size, Vec2f gravity) {
    auto tmp_id = _collidables.peek_next_id();
    ent_to_comp.at(ent).insert(tmp_id);
    comp_to_ent.emplace(tmp_id, ent);
    auto id = notify_created_all(
            create_tmpl(_collidables, position, size, gravity),
            _collision_system,
            _attach_system);
    return id;
}

ID<Trigger> World::create_trigger(EntityID ent) {
    auto tmp_id = _triggers.peek_next_id();
    ent_to_comp.at(ent).insert(tmp_id);
    comp_to_ent.emplace(tmp_id, ent);
    auto id = notify_created_all(
            create_tmpl(_triggers, tmp_id),
            _trigger_system);
    return id;
}

ID<Emitter> World::create_emitter(EntityID ent, EmitterStrategy strat) {
    auto tmp_id = _emitters.peek_next_id();
    ent_to_comp.at(ent).insert(tmp_id);
    comp_to_ent.emplace(tmp_id, ent);
    auto id = notify_created_all(
        create_tmpl(_emitters, strat),
        _emitter_system);
    return id;
}
ID<AttachPoint> World::create_attachpoint(EntityID ent) {
    auto tmp_id = _attachpoints.peek_next_id();
    ent_to_comp.at(ent).insert(tmp_id);
    comp_to_ent.emplace(tmp_id, ent);
    auto id = notify_created_all(
            create_tmpl(_attachpoints, tmp_id),
            _attach_system);
    return id;
}

void World::update(secs deltaTime) {
    if (Level* active = _level_system.get_active(*this))
    {
        _attach_system.update(*this, deltaTime);
        _input.update(deltaTime);
        active->update(*this, deltaTime);
        _trigger_system.update(*this, deltaTime);
        _object_system.update(*this, deltaTime);
        _collision_system.update(*this, deltaTime);
        _camera_system.update(*this, deltaTime);
        _emitter_system.update(*this, deltaTime);

        if (deltaTime > 0.0)
            update_counter++;

        update_time += deltaTime;
    }
}

void World::predraw(float interp, bool updated) {
    if (Level* active = _level_system.get_active(*this))
    {
        _scene_system.set_bg_color(active->getBGColor());
        _scene_system.set_size(active->size());
        _attach_system.predraw(*this, interp, updated);
        _object_system.predraw(*this, interp, updated);
        active->predraw(*this, interp, updated);
        _emitter_system.predraw(*this, interp, updated);
        _scene_system.set_cam_pos(_camera_system.getPosition(interp));
        _scene_system.predraw(*this, interp, updated);
    }
    else
    {
        _scene_system.set_bg_color(ff::Color::Transparent);
    }
}


void World::draw(RenderTarget& target, RenderState state) const
{
    _scene_system.draw(*this, target, state);
}

/*
SurfaceTracker* World::get_tracker(ID<Collidable> collidable_id, ID<SurfaceTracker> tracker_id)
{
    return at(collidable_id).get_tracker(tracker_id);
}

SurfaceTracker& World::at_tracker(ID<Collidable> collidable_id, ID<SurfaceTracker> tracker_id)
{
    return *at(collidable_id).get_tracker(tracker_id);
}
*/

/*
ID<GameObject> World::add_object(copyable_unique_ptr<GameObject>&& obj) {
    return notify_created_all(
            _objects.emplace(std::move(obj)),
            _object_system);
}
*/

ID<Level> World::create_level(const LevelAsset& lvl_asset, bool create_objects) {
    auto tmp_id = _levels.peek_next_id();
    ent_to_comp.emplace(tmp_id, std::set<ComponentID>{});
    auto id = notify_created_all(
            create_tmpl(_levels, *this, tmp_id, lvl_asset),
            _level_system);

    if (create_objects)
    {
        at(id).get_layers().get_obj_layer().createObjectsFromData(*this);
    }
    return id;
}

ID<Level> World::create_level() {
    auto tmp_id = _levels.peek_next_id();
    ent_to_comp.emplace(tmp_id, std::set<ComponentID>{});
    return notify_created_all(
            create_tmpl(_levels, *this, _levels.peek_next_id()),
            _level_system);
}

bool World::erase(EntityID entity) {
    auto components = ent_to_comp.at(entity);
    for (auto& c : components) {
        erase(c);
    }
    auto r = std::visit([this](auto id) { return erase_impl(id); }, entity);
    ent_to_comp.erase(entity);
    return r;
}
bool World::erase(ComponentID component) {
    auto ent = comp_to_ent.at(component);
    auto r = std::visit([this](auto id) { return erase_impl(id); }, component);
    comp_to_ent.erase(component);
    ent_to_comp.at(ent).erase(component);
    return r;
}

bool World::erase_impl(ID<GameObject> id)       { return erase_tmpl(id, _objects, _object_system); }
bool World::erase_impl(ID<Level> id)            { return erase_tmpl(id, _levels, _level_system); }
bool World::erase_impl(ID<Collidable> id)       { return erase_tmpl(id, _collidables, _collision_system, _attach_system); }
bool World::erase_impl(ID<ColliderRegion> id)   { return erase_tmpl(id, _colliders, _collision_system); }
bool World::erase_impl(ID<Emitter> id)          { return erase_tmpl(id, _emitters, _emitter_system); }
bool World::erase_impl(ID<Drawable> id)         { return erase_tmpl(id, _drawables, _scene_system); }
bool World::erase_impl(ID<Trigger> id)          { return erase_tmpl(id, _triggers, _trigger_system); }
bool World::erase_impl(ID<CameraTarget> id)     { return erase_tmpl(id, _camera_targets, _camera_system); }
bool World::erase_impl(ID<AttachPoint> id)      { return erase_tmpl(id, _attachpoints, _attach_system); }

const std::set<ComponentID>& World::get_components_of(EntityID id) const {
    return ent_to_comp.at(id);
}

EntityID World::get_entity_of(ComponentID id) const {
    return comp_to_ent.at(id);
}

std::optional<ID<GameObject>> World::create_object_from_data(ObjectLevelData &data) {
    auto tmp_id = _objects.peek_next_id();
    ent_to_comp.emplace(tmp_id, std::set<ComponentID>{});
    auto ptr = ObjectFactory::createFromData(*this, tmp_id, data);
    if (ptr) {
        return notify_created_all(_objects.emplace(std::move(ptr)), _object_system);
    }
    else {
        ent_to_comp.erase(tmp_id);
        return std::nullopt;
    }
}

}