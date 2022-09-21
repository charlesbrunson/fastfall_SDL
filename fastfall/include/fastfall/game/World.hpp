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

#include <optional>
#include <concepts>
#include <span>

namespace ff {

class World : public Drawable
{
private:
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
            (systems.notify_erased(id), ...);
        }
        return components.erase(id);
    }

    void draw(RenderTarget& target, RenderState state = RenderState()) const override;
public:
    // manage state
    void update(secs deltaTime);
    void predraw(float interp, bool updated);

	// get component
    template<class T>
    T& at(ID<T> id) { return (T&)container<T>().at(id); }

    template<class T>
    const T& at(ID<T> id) const { return (const T&)container<T>().at(id); }

    template<class T>
    T* get(ID<T> id) { return (T*)container<T>().get(id); }

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

    template<class T, class... Args> requires std::derived_from<T, Drawable>
    ID<T> create_object(Args&&... args) {
        std::unique_ptr<T> obj = ObjectFactory::create<T>(this, std::forward<Args>(args)...);
        return notify_created_all(
                poly_create<T>(std::move(obj)),
                _object_system);
    }

    ID<Level> create_level(const LevelAsset& lvl_asset) {
        return notify_created_all(
                create(_levels, this, lvl_asset),
                _level_system) ;
    }

    ID<Level> create_level() {
        return notify_created_all(
                create(_levels, this),
                _level_system) ;
    }

	// erase component
    template<class T>
    bool erase(ID<T> id) { return erase(id, container<T>()); }

    // span components
    template<class T>
    inline auto& all() { return container<T>(); }

	// get system
	inline CollisionSystem& collision() { return _collision_system; }
	inline TriggerSystem&   trigger()   { return _trigger_system; }
	inline CameraSystem&    camera()    { return _camera_system; }
	inline SceneSystem&     scene()     { return _scene_system; }
    inline ObjectSystem&    objects() 	{ return _object_system; }
    inline LevelSystem&     levels()    { return _level_system; }

private:
	// entities
	poly_id_map<GameObject> _objects;
	id_map<Level> 			_levels;

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

    size_t update_counter = 0;
};

}

