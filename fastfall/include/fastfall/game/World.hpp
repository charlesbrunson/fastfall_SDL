#pragma once

#include "fastfall/util/id.hpp"
#include "fastfall/util/id_map.hpp"
#include "fastfall/game/ComponentID.hpp"
#include "fastfall/game/Entity.hpp"

#include "fastfall/game/level/Level.hpp"
#include "fastfall/game/CollisionSystem.hpp"
#include "fastfall/game/CameraSystem.hpp"
#include "fastfall/game/TriggerSystem.hpp"
#include "fastfall/game/SceneSystem.hpp"
#include "fastfall/game/ActorSystem.hpp"
#include "fastfall/game/LevelSystem.hpp"
#include "fastfall/game/EmitterSystem.hpp"
#include "fastfall/game/AttachSystem.hpp"
#include "fastfall/game/PathSystem.hpp"
#include "fastfall/engine/input/InputState.hpp"

#include <optional>
#include <concepts>
#include <span>

namespace ff {

namespace detail {
    template<class T, class Container, class Item = typename Container::base_type>
    constexpr bool fits =
                (std::same_as<id_map<Item>,      Container>      && std::same_as<T, Item>)
             || (std::same_as<poly_id_map<Item>, Container> && std::derived_from<T, Item>);
}


class World : public Drawable
{
private:
    struct state_t
    {
        // entity
        id_map<Entity> _entities;
        std::unordered_map<ComponentID, ID<Entity>> _comp_to_ent;
        //std::unordered_map<ID<Actor>, ID<Entity>>   _actor_to_ent;

        // components
        poly_id_map<Actor>          _actors;
        id_map<Collidable>          _collidables;
        poly_id_map<ColliderRegion> _colliders;
        id_map<Trigger>             _triggers;
        poly_id_map<CameraTarget>   _camera_targets;
        poly_id_map<Drawable>       _drawables;
        id_map<Emitter>             _emitters;
        id_map<AttachPoint>         _attachpoints;
        id_map<PathMover>           _pathmovers;
        id_map<TileLayer>           _tilelayer;

        std::vector<ID<Drawable>>   erase_drawables_deferred;

        // systems
        LevelSystem     _level_system;
        ActorSystem     _actor_system;
        CollisionSystem _collision_system;
        TriggerSystem   _trigger_system;
        EmitterSystem   _emitter_system;
        AttachSystem    _attach_system;
        CameraSystem    _camera_system;
        SceneSystem     _scene_system;
        PathSystem      _path_system;

        constexpr auto all_systems()  {
            return std::tie(
                _level_system, _actor_system, _collision_system, _trigger_system, _emitter_system,
                _attach_system, _camera_system, _scene_system, _path_system
            );
        }

        // misc
        size_t update_counter = 0;
        secs update_time = 0.0;

        // input
        InputState _input;
        InputSourceNull input_null;
    };
    state_t state;

private:
    template<class T>
    constexpr auto& list_for() const {
        return const_cast<World&>(*this).list_for<T>();
    }

    template<class T>
    constexpr auto& list_for() {
        //if constexpr (detail::fits<T, decltype(state._objects)>)        { return state._objects; }
        //if constexpr (detail::fits<T, decltype(state._levels)>)         { return state._levels; }
        if constexpr (detail::fits<T, decltype(state._actors)>)         { return state._actors; }
        if constexpr (detail::fits<T, decltype(state._collidables)>)    { return state._collidables; }
        if constexpr (detail::fits<T, decltype(state._colliders)>)      { return state._colliders; }
        if constexpr (detail::fits<T, decltype(state._triggers)>)       { return state._triggers; }
        if constexpr (detail::fits<T, decltype(state._camera_targets)>) { return state._camera_targets; }
        if constexpr (detail::fits<T, decltype(state._drawables)>)      { return state._drawables; }
        if constexpr (detail::fits<T, decltype(state._emitters)>)       { return state._emitters; }
        if constexpr (detail::fits<T, decltype(state._attachpoints)>)   { return state._attachpoints; }
        if constexpr (detail::fits<T, decltype(state._pathmovers)>)     { return state._pathmovers; }
        if constexpr (detail::fits<T, decltype(state._tilelayer)>)      { return state._tilelayer; }
    }

public:
    World();
    World(const World&);
    World(World&&) noexcept;
    World& operator=(const World&);
    World& operator=(World&&) noexcept;
    ~World() override;

    // manage state
    void update(secs deltaTime);
    void predraw(float interp, bool updated) override;

	// access component
    template<class T>
    T& at(ID<T> id) { return list_for<T>().at(id); }

    template<class T>
    const T& at(ID<T> id) const { return list_for<T>().at(id); }

    template<class... IDs>
    requires (sizeof...(IDs) > 1)
    auto at(IDs... ids) { return std::forward_as_tuple(at(ids)...); }

    template<class... IDs>
    requires (sizeof...(IDs) > 1)
    auto at(IDs... ids) const { return std::forward_as_tuple(at(ids)...); }

    template<class T>
    T* get(ID<T> id) { return list_for<T>().get(id); }

    template<class T>
    const T* get(ID<T> id) const { return list_for<T>().get(id); }

    template<class T>
    T* get(std::optional<ID<T>> id) { return id ? list_for<T>().get(*id) : nullptr; }

    template<class T>
    const T* get(std::optional<ID<T>> id) const { return id ? list_for<T>().get(*id) : nullptr; }

    template<class... IDs>
    requires (sizeof...(IDs) > 1)
    auto get(IDs... ids) { return std::forward_as_tuple(get(ids)...); }

    template<class... IDs>
    requires (sizeof...(IDs) > 1)
    auto get(IDs... ids) const { return std::forward_as_tuple(get(ids)...); }

    // create entity
    std::optional<ID<Entity>> create_entity();

    template<class T_Actor, class... Args>
    requires valid_actor_ctor<T_Actor, Args...>
    std::optional<ID_ptr<T_Actor>> create_actor_entity(Args&&... args) {
        auto id = create_entity();
        if (id) {
            if (create_actor<T_Actor>(*id, std::forward<Args>(args)...)) {
                auto actor_id = id_cast<T_Actor>(*state._entities.at(*id).actor);
                system_notify_created<Actor>(actor_id);
                return ID_ptr<T_Actor>{actor_id, get(actor_id) };
            }
            else {
                erase(*id);
                LOG_ERR_("failed to initialize entity");
            }
        }
        return std::nullopt;
    }

    std::optional<ID_ptr<Object>> create_object(ObjectLevelData& data);

    void reset_entity(ID<Entity> id);

    template<class T_Actor, class... Args>
    requires valid_actor_ctor<T_Actor, Args...>
    std::optional<ID<Entity>> reset_entity(ID<Entity> id, Args&&... args) {
        reset_entity(id);
        if (!create_actor<T_Actor>(id, std::forward<Args>(args)...)) {
            reset_entity(id);
            LOG_ERR_("failed to initialize entity");
            return std::nullopt;
        }
        return id;
    }


    // create component
    template<typename T>
    ID_ptr<T> create(ID<Entity> ent, auto&&... args) {
        auto& list = list_for<T>();
        using container_t = std::remove_cvref_t<decltype(list)>;
        using base_type = typename container_t::base_type;
        ID<T> tmp_id = id_cast<T>(list.peek_next_id());
        tie_component_entity(tmp_id, ent);
        if constexpr (container_t::is_poly) {
            tmp_id = list.template create<T>(std::forward<swap_id_t<decltype(args), ID<T>>>(
                    set_placeholder_id(std::forward<decltype(args)>(args), tmp_id))...);
        }
        else {
            tmp_id = list.create(std::forward<swap_id_t<decltype(args), ID<T>>>(
                    set_placeholder_id(std::forward<decltype(args)>(args), tmp_id))...);
        }
        system_notify_created<base_type>(tmp_id);
        return { tmp_id, get(tmp_id) };
    }


	// erase component
    bool erase(ID<Entity> entity_id);
    bool erase(ComponentID component_id);
    bool erase_all_components(ID<Entity> entity_id);

    // span components
    template<class T>
    inline auto& all() { return list_for<T>(); }
    template<class T>
    inline const auto& all() const { return list_for<T>(); }

	// access system
    template<class T>
    inline constexpr T& system() { return std::get<T&>(state.all_systems()); }

    // entity helpers
    const std::set<ComponentID>& components_of(ID<Entity> id) const;
    ID<Entity> entity_of(ComponentID id) const;
    //ID<Entity> entity_of(ID<Actor> id) const;

    template<std::derived_from<Actor> T_Actor>
    ID<T_Actor> id_of_actor(T_Actor* actor) const {
        return id_cast<T_Actor>(*state._entities.at(actor->entity_id()).actor);
    }

    bool entity_has_actor(ID<Entity> id) const;

    // misc
    std::string name;
    inline size_t tick_count() const { return state.update_counter; }
    inline secs uptime() const { return state.update_time; }
    inline InputState& input() { return state._input; }

    bool due_to_erase(ID<Drawable> id) const;

private:
    void draw(RenderTarget& target, RenderState state = RenderState()) const override;

    template<class T_Actor, class... Args>
    requires valid_actor_ctor<T_Actor, Args...>
    bool create_actor(ID<Entity> id, Args&&... args) {
        auto& ent = state._entities.at(id);

        ent.actor = state._actors.create<T_Actor>(ActorInit{
            .world = *this,
            .entity_id = id,
            .actor_id = state._actors.peek_next_id()
        }, std::forward<Args>(args)...);
        return at(*ent.actor).initialized;
    }


    template<typename T>
    void system_notify_created(ID<T> t_id) {
        std::apply([&, this](auto&... system) {
            ([&, this]<typename System>(System& sys){
                if constexpr (requires(System s, ID<T> i, World& w) { s.notify_created(w, i); }) {
                    sys.notify_created(*this, t_id);
                }
            }(system), ...);
        }, state.all_systems());
    }

    template<typename T>
    void system_notify_erased(ID<T> t_id) {
        std::apply([&, this](auto&... system) {
            ([&, this]<typename System>(System& sys){
                if constexpr (requires(System s, ID<T> i, World& w) { s.notify_erased(w, i); }) {
                    sys.notify_erased(*this, t_id);
                }
            }(system), ...);
        }, state.all_systems());
    }

    void tie_component_entity(ComponentID cmp, ID<Entity> ent);
    void untie_component_entity(ComponentID cmp, ID<Entity> ent);

    void clean_drawables();
};

}

