#include "fastfall/game/World.hpp"

namespace ff {

ID<Collidable> World::create_collidable(Vec2f position, Vec2f size, Vec2f gravity) {
    return notify_created_all(
            create(_collidables, position, size, gravity),
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
    if (Level* active = _level_system.get_active(*this))
    {
        active->update(*this, deltaTime);
        _trigger_system.update(*this, deltaTime);
        _object_system.update(*this, deltaTime);
        _collision_system.update(*this, deltaTime);
        _camera_system.update(*this, deltaTime);
        update_counter++;
    }
}

void World::predraw(float interp, bool updated) {
    // TODO
    //if (want_reset)
    //    reset();

    if (Level* active = _level_system.get_active(*this))
    {
        _scene_system.set_bg_color(active->getBGColor());
        _scene_system.set_size(active->size());
        _object_system.predraw(*this, interp, updated);
        active->predraw(*this, interp, updated);
        _scene_system.set_cam_pos(_camera_system.getPosition(interp));
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


}