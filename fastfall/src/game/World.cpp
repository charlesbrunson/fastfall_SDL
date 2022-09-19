#include "fastfall/game/World.hpp"

namespace ff {

World::World() {
    _collision_system.set_world(this);
    _trigger_system.set_world(this);
    _camera_system.set_world(this);
    _scene_system.set_world(this);
    _object_system.set_world(this);
}

ID<Collidable> World::create_collidable(Vec2f position, Vec2f size, Vec2f gravity) {
    return notify_created_all(
            create(_collidables, position, size, gravity),
            _collision_system);
}

ID<SurfaceTracker> World::create_tracker(ID<Collidable> collidable, Angle ang_min, Angle ang_max, bool inclusive) {
    return notify_created_all(
            create(_trackers, collidable, ang_min, ang_max, inclusive),
            _collision_system);
}

ID<Trigger> World::create_trigger() {
    return notify_created_all(
            create(_triggers),
            _trigger_system);
}

ID<SceneObject> World::create_scene_object(SceneObject obj) {
    return notify_created_all(
            create(_scene_objects, obj),
            _scene_system);
}

void World::update(secs deltaTime) {
    if (Level* active = _level_system.get_active())
    {
        active->update(deltaTime);
        _trigger_system.update(deltaTime);
        _object_system.update(deltaTime);
        _collision_system.update(deltaTime);
        _camera_system.update(deltaTime);
        update_counter++;
    }
}

void World::predraw(float interp, bool updated) {
    // TODO
    //if (want_reset)
    //    reset();

    if (Level* active = _level_system.get_active())
    {
        _scene_system.set_bg_color(active->getBGColor());
        _scene_system.set_size(active->size());
        _object_system.predraw(interp, updated);
        active->predraw(interp, updated);
        _scene_system.set_cam_pos(_camera_system.getPosition(interp));
    }
    else
    {
        _scene_system.set_bg_color(ff::Color::Transparent);
    }
}

}