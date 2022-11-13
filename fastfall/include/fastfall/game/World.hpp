#pragma once

#include "fastfall/util/id.hpp"
#include "fastfall/util/id_map.hpp"
#include "fastfall/game/AnyID.hpp"

#include "fastfall/game/level/Level.hpp"
#include "fastfall/game/CollisionSystem.hpp"
#include "fastfall/game/CameraSystem.hpp"
#include "fastfall/game/TriggerSystem.hpp"
#include "fastfall/game/SceneSystem.hpp"
#include "fastfall/game/ObjectSystem.hpp"
#include "fastfall/game/LevelSystem.hpp"
#include "fastfall/game/EmitterSystem.hpp"
#include "fastfall/game/AttachSystem.hpp"
#include "fastfall/engine/input/InputState.hpp"

#include <optional>
#include <concepts>
#include <span>

namespace ff {


class World : public Drawable
{
private:
    template<class T>
    constexpr auto& container() const {
        if      constexpr (std::same_as<T, Collidable>)          { return _collidables; }
        else if constexpr (std::same_as<T, Trigger>)             { return _triggers; }
        else if constexpr (std::same_as<T, Emitter>)             { return _emitters; }
        else if constexpr (std::same_as<T, AttachPoint>)         { return _attachpoints; }
        else if constexpr (std::same_as<T, Level>)               { return _levels; }
        else if constexpr (std::derived_from<T, ColliderRegion>) { return _colliders; }
        else if constexpr (std::derived_from<T, CameraTarget>)   { return _camera_targets; }
        else if constexpr (std::derived_from<T, Drawable>)       { return _drawables; }
        else if constexpr (std::derived_from<T, GameObject>)     { return _objects; }
        else { throw std::exception{}; }
    }

    template<class T>
    constexpr auto& container() {
        if      constexpr (std::same_as<T, Collidable>)          { return _collidables; }
        else if constexpr (std::same_as<T, Trigger>)             { return _triggers; }
        else if constexpr (std::same_as<T, Emitter>)             { return _emitters; }
        else if constexpr (std::same_as<T, AttachPoint>)         { return _attachpoints; }
        else if constexpr (std::same_as<T, Level>)               { return _levels; }
        else if constexpr (std::derived_from<T, ColliderRegion>) { return _colliders; }
        else if constexpr (std::derived_from<T, CameraTarget>)   { return _camera_targets; }
        else if constexpr (std::derived_from<T, Drawable>)       { return _drawables; }
        else if constexpr (std::derived_from<T, GameObject>)     { return _objects; }
        else { throw std::exception{}; }
    }

    auto span(auto& components) { return std::span{components.begin(), components.end()}; };

    auto create_tmpl(auto& container, auto&&... args) {
        return container.create(std::forward<decltype(args)>(args)...);
    }

    template<class T>
    auto poly_create_tmpl(auto& container, auto&&... args) {
        return container.template create<T>(std::forward<decltype(args)>(args)...);
    }

    auto notify_created_all(auto id, auto&... systems) {
        (systems.notify_created(*this, id), ...);
        return id;
    }

    bool erase_tmpl(auto id, auto& components, auto&... systems) {
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

    std::string name;

    // manage state
    void update(secs deltaTime);
    void predraw(float interp, bool updated);

	// access component
    template<class T>
    T& at(ID<T> id) { return container<T>().at(id); }

    template<class T>
    const T& at(ID<T> id) const { return container<T>().at(id); }

    template<class... IDs>
    requires (sizeof...(IDs) > 1)
    auto at(IDs... ids) { return std::forward_as_tuple(at(ids)...); }

    template<class... IDs>
    requires (sizeof...(IDs) > 1)
    auto at(IDs... ids) const { return std::forward_as_tuple(at(ids)...); }

    template<class T>
    T* get(ID<T> id) { return container<T>().get(id); }

    template<class T>
    const T* get(ID<T> id) const { return container<T>().get(id); }

    template<class... IDs>
    requires (sizeof...(IDs) > 1)
    auto get(IDs... ids) { return std::forward_as_tuple(get(ids)...); }

    template<class... IDs>
    requires (sizeof...(IDs) > 1)
    auto get(IDs... ids) const { return std::forward_as_tuple(get(ids)...); }

    // create entity
    std::optional<ID<GameObject>> create_object_from_data(ObjectLevelData& data);
    template<class T, class... Args> requires std::derived_from<T, GameObject>
    ID<T> create_object(Args&&... args) {
        ent_to_comp.emplace(_objects.peek_next_id(), std::set<ComponentID>{});
        auto id = notify_created_all(
                poly_create_tmpl<T>(_objects, *this, _objects.peek_next_id(), std::forward<Args>(args)...),
                _object_system);
        return id;
    }

    ID<Level> create_level(const LevelAsset& lvl_asset, bool create_objects);
    ID<Level> create_level();

    // create component
	ID<Collidable> create_collidable(EntityID ent, Vec2f position, Vec2f size, Vec2f gravity = Vec2f{});
    ID<Trigger> create_trigger(EntityID ent);
    ID<AttachPoint> create_attachpoint(EntityID ent);
    ID<Emitter> create_emitter(EntityID ent, EmitterStrategy strat = {});

    template<class T, class... Args> requires std::derived_from<T, ColliderRegion>
	ID<T> create_collider(EntityID ent, Args&&... args) {
        auto tmp_id = _colliders.peek_next_id();
        ent_to_comp.at(ent).insert(tmp_id);
        comp_to_ent.emplace(tmp_id, ent);
        auto id = notify_created_all(
                poly_create_tmpl<T>(_colliders, std::forward<Args>(args)...),
                _collision_system);
        return id;
    }

	template<class T, class... Args> requires std::derived_from<T, CameraTarget>
	ID<T> create_camera_target(EntityID ent, Args&&... args) {
        auto tmp_id = _camera_targets.peek_next_id();
        ent_to_comp.at(ent).insert(tmp_id);
        comp_to_ent.emplace(tmp_id, ent);
        auto id = notify_created_all(
                poly_create_tmpl<T>(_camera_targets, std::forward<Args>(args)...),
                _camera_system) ;
        return id;
    }

    template<class T, class... Args> requires std::derived_from<T, Drawable>
    ID<T> create_drawable(EntityID ent, Args&&... args) {
        auto tmp_id = _drawables.peek_next_id();
        ent_to_comp.at(ent).insert(tmp_id);
        comp_to_ent.emplace(tmp_id, ent);
        auto id = notify_created_all(
                poly_create_tmpl<T>(_drawables, std::forward<Args>(args)...),
                _scene_system);
        return id;
    }

	// erase component
    bool erase(EntityID entity);
    bool erase(ComponentID component);

    // span components
    template<class T>
    inline auto& all() { return container<T>(); }

	// access system
	inline CollisionSystem& collision() { return _collision_system; }
	inline TriggerSystem&   trigger()   { return _trigger_system; }
	inline CameraSystem&    camera()    { return _camera_system; }
	inline SceneSystem&     scene()     { return _scene_system; }
    inline AttachSystem&    attach()    { return _attach_system; }
    inline InputState&      input()     { return _input; }

    inline ObjectSystem&    objects() 	{ return _object_system; }
    inline LevelSystem&     levels()    { return _level_system; }

    size_t tick_count() const { return update_counter; }
    secs uptime() const { return update_time; }

    const std::set<ComponentID>& get_components_of(EntityID id) const;
    EntityID get_entity_of(ComponentID id) const;

private:
    void draw(RenderTarget& target, RenderState state = RenderState()) const override;

    bool erase_impl(ID<GameObject> id);
    bool erase_impl(ID<Level> id);
    bool erase_impl(ID<Collidable> id);
    bool erase_impl(ID<ColliderRegion> id);
    bool erase_impl(ID<Emitter> id);
    bool erase_impl(ID<Drawable> id);
    bool erase_impl(ID<Trigger> id);
    bool erase_impl(ID<CameraTarget> id);
    bool erase_impl(ID<AttachPoint> id);

	// entities
	poly_id_map<GameObject> _objects;
	id_map<Level> 			_levels;

	// components
	id_map<Collidable> 			_collidables;
	poly_id_map<ColliderRegion> _colliders;
	id_map<Trigger> 			_triggers;
	poly_id_map<CameraTarget> 	_camera_targets;
    poly_id_map<Drawable>       _drawables;
    id_map<Emitter>             _emitters;
    id_map<AttachPoint>         _attachpoints;

    // entity to component lookup
    std::unordered_map<EntityID, std::set<ComponentID>> ent_to_comp;
    std::unordered_map<ComponentID, EntityID>           comp_to_ent;

	// systems
	LevelSystem	    _level_system;
	ObjectSystem	_object_system;
	CollisionSystem _collision_system;
	TriggerSystem	_trigger_system;
    EmitterSystem	_emitter_system;
    AttachSystem	_attach_system;
	CameraSystem	_camera_system;
	SceneSystem		_scene_system;
    InputState      _input;

    size_t update_counter = 0;
    secs update_time = 0.0;

    InputSourceNull input_null;
};

}

