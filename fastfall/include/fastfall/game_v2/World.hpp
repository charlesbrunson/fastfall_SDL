#pragma once

#include "fastfall/util/id.hpp"
#include "fastfall/util/id_map.hpp"

#include "fastfall/game_v2/CollisionSystem.hpp"
#include "fastfall/game_v2/CameraSystem.hpp"
#include "fastfall/game_v2/TriggerSystem.hpp"
#include "fastfall/game_v2/SceneSystem.hpp"

#include <optional>
#include <concepts>
#include <span>

namespace ff {

class World
{
private:
    auto span(auto& components) { return std::span{components.begin(), components.end()}; };

    auto create(auto& components, auto&&... args) {
        auto id = components.create(std::forward<decltype(args)>(args)...);
        components.at(id).set_id(id);
        return id;
    }

    template<class T>
    auto poly_create(auto& components, auto&&... args) {
        auto id = components.template create<T>(std::forward<decltype(args)>(args)...);
        components.at(id).set_id(id);
        return id;
    }

    auto notify_created_all(auto id, auto&... systems) {
        (systems.notify_created(id), ...);
        return id;
    }

    bool erase(auto id, auto& components, auto&... systems) {
        bool exists = components.exists(id);
        if (exists) {
            (systems.notify_erased(id), ...);
        }
        return components.erase(id);
    }

public:
    // manage state
    // void update(secs deltaTime)

	// get entity
	//GameObject* get(ID<GameObject> id);	
	//Level* 		get(ID<Level> id);	
	//Level* 		get_active_level();	

	// create entity
	
	// erase entity

	// get component
    Collidable&     operator[](ID<Collidable> id)   { return _collidables.at(id); }
    SurfaceTracker& operator[](ID<SurfaceTracker> id) { return _trackers.at(id); }
    ColliderRegion& operator[](ID<ColliderRegion> id) { return _colliders.at(id); }
	Trigger&        operator[](ID<Trigger> id)      { return _triggers.at(id); }
	CameraTarget&   operator[](ID<CameraTarget> id) { return _camera_targets.at(id); }
	Drawable&       operator[](ID<Drawable> id)     { return _drawables.at(id); }
    SceneObject&    operator[](ID<SceneObject> id)  { return _scene_objects.at(id); }

    Collidable&     at(ID<Collidable> id)   { return _collidables.at(id); }
    SurfaceTracker& at(ID<SurfaceTracker> id) { return _trackers.at(id); }
    ColliderRegion& at(ID<ColliderRegion> id) { return _colliders.at(id); }
    Trigger&        at(ID<Trigger> id)      { return _triggers.at(id); }
    CameraTarget&   at(ID<CameraTarget> id) { return _camera_targets.at(id); }
    Drawable&       at(ID<Drawable> id)     { return _drawables.at(id); }
    SceneObject&    at(ID<SceneObject> id)  { return _scene_objects.at(id); }

    //Collidable* 	get(ID<Collidable> id);
    //ColliderRegion* get(ID<ColliderRegion> id);
    Collidable*     get(ID<Collidable> id)  { return _collidables.get(id); }
    SurfaceTracker* get(ID<SurfaceTracker> id) { return _trackers.get(id); }
    ColliderRegion* get(ID<ColliderRegion> id) { return _colliders.get(id); }
    Trigger*        get(ID<Trigger> id)     { return _triggers.get(id); }
    CameraTarget*   get(ID<CameraTarget> id){ return _camera_targets.get(id); }
    Drawable*       get(ID<Drawable> id)    { return _drawables.get(id); }
    SceneObject*    get(ID<SceneObject> id) { return _scene_objects.get(id); }

	// create component
    template<class... Args>
	ID<Collidable> create_collidable(Args&&... args) {
        return notify_created_all(
                create(_collidables, std::forward<Args>(args)...),
                _collision_system);
    }

    template<class... Args>
    ID<SurfaceTracker> create_tracker(ID<Collidable> collidable, Args&&... args) {
        return notify_created_all(
                create(_collidables, collidable, std::forward<Args>(args)...),
                _collision_system);
    }

    template<class T, class... Args> requires std::derived_from<T, ColliderRegion>
	ID<T> create_collider(Args&&... args) {
        return notify_created_all(
                poly_create<T>(_colliders, std::forward<Args>(args)...),
                _collision_system);
    }

    template<class... Args>
	ID<Trigger> create_trigger(Args&&... args) {
        return notify_created_all(
                create(_triggers, std::forward<Args>(args)...),
                _trigger_system);
    }

	template<class T, class... Args> requires std::derived_from<T, CameraTarget>
	ID<T> create_camera_target(Args&&... args) {
        return notify_created_all(
                poly_create<T>(_camera_targets, std::forward<Args>(args)...),
                _camera_system) ;
    }

    template<class T, class... Args> requires std::derived_from<T, Drawable>
	ID<T> create_drawable(Args&&... args) {
        return notify_created_all(
                poly_create<T>(_drawables, std::forward<Args>(args)...));
    }

    template<class... Args>
    ID<SceneObject> create_scene_object(Args&&... args) {
        return notify_created_all(
                create(_scene_objects, std::forward<Args>(args)...),
                _scene_system);
    }

	// erase component
	bool erase(ID<Collidable> id)        { return erase(id, _collidables, _collision_system); }
    bool erase(ID<SurfaceTracker> id)       { return erase(id, _trackers, _collision_system); }
	bool erase(ID<ColliderRegion> id)      { return erase(id, _colliders, _collision_system); }
	bool erase(ID<Trigger> id)              { return erase(id, _triggers, _trigger_system); }
	bool erase(ID<CameraTarget> id)   { return erase(id, _camera_targets, _camera_system); }
	bool erase(ID<Drawable> id)            { return erase(id, _drawables); }
    bool erase(ID<SceneObject> id)     { return erase(id, _scene_objects, _scene_system); }

    // span components
    inline auto& all_collidables() { return _collidables; }
    inline auto& all_trackers() { return _trackers; }
    inline auto& all_colliders() { return _colliders; }
    inline auto& all_triggers() { return _triggers; }
    inline auto& all_camera_targets() { return _camera_targets; }
    inline auto& all_drawables() { return _drawables; }
    inline auto& all_scene_objects() { return _scene_objects; }

	// get system
	//LevelSystem& 		levels() 	{ return _level_system; }
	//ObjectSystem& 		objects() 	{ return _object_system; }
	inline CollisionSystem& collision() { return _collision_system; }
	inline TriggerSystem&   trigger()   { return _trigger_system; }
	inline CameraSystem&    camera()    { return _camera_system; }
	inline SceneSystem&     scene()     { return _scene_system; }

private:
	// entities
	//poly_id_map<GameObject> _objects;
	//id_map<Level> 			_levels;

	// components
	id_map<Collidable> 			_collidables;
    id_map<SurfaceTracker> 		_trackers;
	poly_id_map<ColliderRegion> _colliders;
	id_map<Trigger> 			_triggers;
	poly_id_map<CameraTarget> 	_camera_targets;
	poly_id_map<Drawable> 		_drawables;
    id_map<SceneObject>         _scene_objects;

	// systems
	//LevelSystem	_level_system;
	//ObjectSystem	_object_system;
	CollisionSystem _collision_system;
	TriggerSystem	_trigger_system;
	CameraSystem	_camera_system;
	SceneSystem		_scene_system;
};

}

