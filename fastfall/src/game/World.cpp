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
    _level_system = other._level_system;
    _object_system = other._object_system;
    _collision_system = other._collision_system;
    _trigger_system = other._trigger_system;
    _camera_system = other._camera_system;
    _scene_system = other._scene_system;
    _emitter_system = other._emitter_system;
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
    _level_system = std::move(other._level_system);
    _object_system = std::move(other._object_system);
    _collision_system = std::move(other._collision_system);
    _trigger_system = std::move(other._trigger_system);
    _camera_system = std::move(other._camera_system);
    _scene_system = std::move(other._scene_system);
    _emitter_system = std::move(other._emitter_system);
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
    _level_system = other._level_system;
    _object_system = other._object_system;
    _collision_system = other._collision_system;
    _trigger_system = other._trigger_system;
    _camera_system = other._camera_system;
    _scene_system = other._scene_system;
    _emitter_system = other._emitter_system;
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
    _level_system = std::move(other._level_system);
    _object_system = std::move(other._object_system);
    _collision_system = std::move(other._collision_system);
    _trigger_system = std::move(other._trigger_system);
    _camera_system = std::move(other._camera_system);
    _scene_system = std::move(other._scene_system);
    _emitter_system = std::move(other._emitter_system);
    update_counter = other.update_counter;
    update_time = other.update_time;
    return *this;
}

World::~World() {
    WorldImGui::remove(this);
}

ID<Collidable> World::create_collidable(Vec2f position, Vec2f size, Vec2f gravity) {
    return notify_created_all(
            create(_collidables, position, size, gravity),
            _collision_system);
}

ID<Trigger> World::create_trigger() {
    return notify_created_all(
            create(_triggers, _triggers.peek_next_id()),
            _trigger_system);
}

ID<Emitter> World::create_emitter(EmitterStrategy strat) {
    return notify_created_all(
        create(_emitters, strat),
        _emitter_system);
}

/*
ID<SceneObject> World::create_scene_object(SceneObject obj) {
    return notify_created_all(
            create(_scene_objects, obj),
            _scene_system);
}
*/

void World::update(secs deltaTime) {
    if (Level* active = _level_system.get_active(*this))
    {
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
    auto id = notify_created_all(
            create(_levels, *this, _levels.peek_next_id(), lvl_asset),
            _level_system);

    if (create_objects)
    {
        at(id).get_layers().get_obj_layer().createObjectsFromData(*this);
    }

    return id;
}

ID<Level> World::create_level() {
    return notify_created_all(
            create(_levels, *this, _levels.peek_next_id()),
            _level_system);
}

bool World::erase(ID<GameObject> id)       { return erase(id, _objects, _object_system); }
bool World::erase(ID<Level> id)            { return erase(id, _levels, _level_system); }
bool World::erase(ID<Collidable> id)       { return erase(id, _collidables, _collision_system); }
bool World::erase(ID<ColliderRegion> id)   { return erase(id, _colliders, _collision_system); }
bool World::erase(ID<Emitter> id)          { return erase(id, _emitters, _emitter_system); }
bool World::erase(ID<Drawable> id)         { return erase(id, _drawables, _scene_system); }
bool World::erase(ID<Trigger> id)          { return erase(id, _triggers, _trigger_system); }
bool World::erase(ID<CameraTarget> id)     { return erase(id, _camera_targets, _camera_system); }

std::optional<ID<GameObject>> World::create_object_from_data(ObjectLevelData &data) {
    auto ptr = ObjectFactory::createFromData(*this, _objects.peek_next_id(), data);
    if (ptr) {
        return notify_created_all(_objects.emplace(std::move(ptr)), _object_system);
    }
    else {
        return std::nullopt;
    }
}

}