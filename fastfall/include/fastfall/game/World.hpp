#pragma once

#include "fastfall/util/id.hpp"
#include "fastfall/util/id_map.hpp"

#include "fastfall/game/level/Level.hpp"
#include "fastfall/game/CollisionSystem.hpp"
#include "fastfall/game/CameraSystem.hpp"
#include "fastfall/game/TriggerSystem.hpp"
#include "fastfall/game/SceneSystem.hpp"
#include "fastfall/game/ObjectSystem.hpp"
#include "fastfall/game/LevelSystem.hpp"
#include "fastfall/engine/InputState.hpp"

#include <optional>
#include <concepts>
#include <span>

namespace ff {

class World : public Drawable
{
private:
    template<class T>
    constexpr auto& container() const {
        if constexpr (std::same_as<T, Collidable>) {
            return _collidables;
        }
        else if constexpr (std::derived_from<T, ColliderRegion>) {
            return _colliders;
        }
        else if constexpr (std::same_as<T, Trigger>) {
            return _triggers;
        }
        else if constexpr (std::derived_from<T, CameraTarget>) {
            return _camera_targets;
        }
        else if constexpr (std::same_as<T, SceneObject>) {
            return _scene_objects;
        }
        else if constexpr (std::derived_from<T, GameObject>) {
            return _objects;
        }
        else if constexpr (std::same_as<T, Level>) {
            return _levels;
        }
        else { throw std::exception{}; }
    }

    template<class T>
    constexpr auto& container() {
        if constexpr (std::same_as<T, Collidable>) {
            return _collidables;
        }
        else if constexpr (std::derived_from<T, ColliderRegion>) {
            return _colliders;
        }
        else if constexpr (std::same_as<T, Trigger>) {
            return _triggers;
        }
        else if constexpr (std::derived_from<T, CameraTarget>) {
            return _camera_targets;
        }
        else if constexpr (std::same_as<T, SceneObject>) {
            return _scene_objects;
        }
        else if constexpr (std::derived_from<T, GameObject>) {
            return _objects;
        }
        else if constexpr (std::same_as<T, Level>) {
            return _levels;
        }
        else { throw std::exception{}; }
    }

    auto span(auto& components) { return std::span{components.begin(), components.end()}; };

    auto create(auto& container, auto&&... args) {
        return container.create(std::forward<decltype(args)>(args)...);
    }

    template<class T>
    auto poly_create(auto& container, auto&&... args) {
        return container.template create<T>(std::forward<decltype(args)>(args)...);
    }

    auto notify_created_all(auto id, auto&... systems) {
        (systems.notify_created(*this, id), ...);
        return id;
    }

    bool erase(auto id, auto& components, auto&... systems) {
        bool exists = components.exists(id);
        if (exists) {
            (systems.notify_erased(*this, id), ...);
        }
        return components.erase(id);
    }

public:
    World();
    World(const World&);
    World(World&&) noexcept;
    World& operator=(const World&);
    World& operator=(World&&) noexcept;
    ~World();

    // manage state
    void update(secs deltaTime);
    void predraw(float interp, bool updated);

	// access component
    template<class T>
    T& at(ID<T> id) { return (T&)container<T>().at(id); }

    template<class T>
    const T& at(ID<T> id) const { return (T&)container<T>().at(id); }

    template<class... IDs>
    requires (sizeof...(IDs) > 1)
    auto at(IDs... ids) { return std::forward_as_tuple(at(ids)...); }

    template<class... IDs>
    requires (sizeof...(IDs) > 1)
    auto at(IDs... ids) const { return std::forward_as_tuple(at(ids)...); }

    template<class T>
    T* get(ID<T> id) { return (T*)container<T>().get(id); }

    template<class T>
    const T* get(ID<T> id) const { return (T*)container<T>().get(id); }

    template<class... IDs>
    requires (sizeof...(IDs) > 1)
    auto get(IDs... ids) { return std::forward_as_tuple(get(ids)...); }

    template<class... IDs>
    requires (sizeof...(IDs) > 1)
    auto get(IDs... ids) const { return std::forward_as_tuple(get(ids)...); }

    // id_cast SceneObject to a drawble type to retrieve that from the sceneobject
    // kinda scuffed but its fun
    template<std::derived_from<Drawable> T>
    T& at(ID<T> id) { return *(T*)at(id_cast<SceneObject>(id)).drawable.get(); }

    template<std::derived_from<Drawable> T>
    const T& at(ID<T> id) const { return *(T*)at(id_cast<SceneObject>(id)).drawable.get(); }

    template<std::derived_from<Drawable> T>
    T* get(ID<T> id) { return (T*)at(id_cast<SceneObject>(id)).drawable.get(); }

    template<std::derived_from<Drawable> T>
    const T* get(ID<T> id) const { return (T*)at(id_cast<SceneObject>(id)).drawable.get(); }

    SurfaceTracker& at_tracker(ID<Collidable> collidable_id, ID<SurfaceTracker> tracker_id);
    SurfaceTracker* get_tracker(ID<Collidable> collidable_id, ID<SurfaceTracker> tracker_id);

	// create component
	ID<Collidable> create_collidable(Vec2f position, Vec2f size, Vec2f gravity = Vec2f{});
    ID<Trigger> create_trigger();
    ID<SceneObject> create_scene_object(SceneObject obj);

    template<class T, class... Args> requires std::derived_from<T, ColliderRegion>
	ID<T> create_collider(Args&&... args) {
        return notify_created_all(
                poly_create<T>(_colliders, std::forward<Args>(args)...),
                _collision_system);
    }

	template<class T, class... Args> requires std::derived_from<T, CameraTarget>
	ID<T> create_camera_target(Args&&... args) {
        return notify_created_all(
                poly_create<T>(_camera_targets, std::forward<Args>(args)...),
                _camera_system) ;
    }


    template<class T, class... Args> requires std::derived_from<T, GameObject>
    ID<T> create_object(Args&&... args) {
        return notify_created_all(
                poly_create<T>(_objects, *this, _objects.peek_next_id(), std::forward<Args>(args)...),
                _object_system) ;
    }

    std::optional<ID<GameObject>> create_object_from_data(ObjectLevelData& data);

    ID<Level> create_level(const LevelAsset& lvl_asset, bool create_objects);
    ID<Level> create_level();

	// erase component
    bool erase(ID<GameObject> id);
    bool erase(ID<Level> id);
    bool erase(ID<Collidable> id);
    bool erase(ID<ColliderRegion> id);
    bool erase(ID<SceneObject> id);
    bool erase(ID<Trigger> id);
    bool erase(ID<CameraTarget> id);

    // span components
    template<class T>
    inline auto& all() { return container<T>(); }

	// access system
	inline CollisionSystem& collision() { return _collision_system; }
	inline TriggerSystem&   trigger()   { return _trigger_system; }
	inline CameraSystem&    camera()    { return _camera_system; }
	inline SceneSystem&     scene()     { return _scene_system; }
    inline ObjectSystem&    objects() 	{ return _object_system; }
    inline LevelSystem&     levels()    { return _level_system; }
    inline InputState&      input()     { return _input; }

    size_t tick_count() const { return update_counter; }
    secs uptime() const { return update_time; }

private:
    void draw(RenderTarget& target, RenderState state = RenderState()) const override;

	// entities
	poly_id_map<GameObject> _objects;
	id_map<Level> 			_levels;

	// components
	id_map<Collidable> 			_collidables;
	poly_id_map<ColliderRegion> _colliders;
	id_map<Trigger> 			_triggers;
	poly_id_map<CameraTarget> 	_camera_targets;
    id_map<SceneObject>         _scene_objects;

	// systems
	LevelSystem	    _level_system;
	ObjectSystem	_object_system;
	CollisionSystem _collision_system;
	TriggerSystem	_trigger_system;
	CameraSystem	_camera_system;
	SceneSystem		_scene_system;
    InputState      _input;

    size_t update_counter = 0;
    secs update_time = 0.0;
};

}

