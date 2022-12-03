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
#include "fastfall/game/ObjectSystem.hpp"
#include "fastfall/game/LevelSystem.hpp"
#include "fastfall/game/EmitterSystem.hpp"
#include "fastfall/game/AttachSystem.hpp"
#include "fastfall/game/PathSystem.hpp"
#include "fastfall/engine/input/InputState.hpp"

#include <optional>
#include <concepts>
#include <span>

namespace ff {


class World : public Drawable
{
private:
    struct state_t
    {
        // entity
        id_map<Entity> _entities;
        std::unordered_map<ComponentID, ID<Entity>> _comp_to_ent;

        // components
        poly_id_map<GameObject> _objects;
        id_map<Level> _levels;
        id_map<Collidable> _collidables;
        poly_id_map<ColliderRegion> _colliders;
        id_map<Trigger> _triggers;
        poly_id_map<CameraTarget> _camera_targets;
        poly_id_map<Drawable> _drawables;
        id_map<Emitter> _emitters;
        id_map<AttachPoint> _attachpoints;
        id_map<PathMover> _pathmovers;
        id_map<TileLayer> _tilelayer;

        constexpr auto all_component_lists() const {
            return std::tie(
                _objects, _levels, _collidables, _colliders, _triggers, _camera_targets,
                _drawables, _emitters, _attachpoints, _pathmovers, _tilelayer
            );
        }

        constexpr auto all_component_lists()  {
            return std::tie(
                _objects, _levels, _collidables, _colliders, _triggers, _camera_targets,
                _drawables, _emitters, _attachpoints, _pathmovers, _tilelayer
            );
        }

        // systems
        LevelSystem _level_system;
        ObjectSystem _object_system;
        CollisionSystem _collision_system;
        TriggerSystem _trigger_system;
        EmitterSystem _emitter_system;
        AttachSystem _attach_system;
        CameraSystem _camera_system;
        SceneSystem _scene_system;
        PathSystem _path_system;

        constexpr auto all_systems() const {
            return std::tie(
                _level_system, _object_system, _collision_system, _trigger_system, _emitter_system,
                _attach_system, _camera_system, _scene_system, _path_system
            );
        }

        constexpr auto all_systems()  {
            return std::tie(
                _level_system, _object_system, _collision_system, _trigger_system, _emitter_system,
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
    template<typename T>
    static constexpr auto& impl_get_best_component_list(auto& first) {
        using Container = std::remove_cvref_t<decltype(first)>;
        using Item      = typename Container::base_type;

        if constexpr (std::same_as<id_map<T>, Container>
                      || std::same_as<poly_id_map<Item>, Container> && std::derived_from<Item, T>)
        {
            return first;
        }
        else {
            []<bool flag = false>()
            { static_assert(flag, "no match component list"); }();
            return id_map<T>{};
        }
    }

    template<typename T>
    static constexpr auto& impl_get_best_component_list(auto& first, auto&... rest) {
        using Container = std::remove_cvref_t<decltype(first)>;
        using Item      = typename Container::base_type;

        if constexpr (std::same_as<id_map<T>, Container>
                   || std::same_as<poly_id_map<Item>, Container> && std::derived_from<T, Item>)
        {
            return first;
        }
        else {
            return impl_get_best_component_list<T>(rest...);
        }
    }

    template<typename T>
    constexpr auto& list_for() const {
        return std::apply(
                [](auto&... lists) -> auto& { return impl_get_best_component_list<T>(lists...); },
                state.all_component_lists());
    }

    template<class T>
    constexpr auto& list_for() {
        return std::apply(
                [](auto&... lists) -> auto& { return impl_get_best_component_list<T>(lists...); },
                state.all_component_lists());
    }

    auto span(auto& components) { return std::span{components.begin(), components.end()}; };

public:
    World();
    World(const World&);
    World(World&&) noexcept;
    World& operator=(const World&);
    World& operator=(World&&) noexcept;
    ~World();


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
    ID<Entity> create_entity();

    std::optional<ID<GameObject>> create_object_from_data(ObjectLevelData& data);

    ID<Level> create_level(const LevelAsset& levelData, bool create_objects);
    ID<Level> create_level();

    // create component
    template<typename T>
    ID<T> create(ID<Entity> ent, auto&&... args) {
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
        return tmp_id;
    }

	// erase component
    bool erase(ID<Entity> entity);
    bool erase(ComponentID component);

    // span components
    template<class T>
    inline auto& all() { return list_for<T>(); }

	// access system
    template<class T>
    inline constexpr T& system() { return std::get<T&>(state.all_systems()); }

    // entity helpers
    const std::set<ComponentID>& components_of(ID<Entity> id) const;
    ID<Entity> entity_of(ComponentID id) const;

    // misc
    std::string name;
    inline size_t tick_count() const { return state.update_counter; }
    inline secs uptime() const { return state.update_time; }
    inline InputState& input() { return state._input; }

private:
    void draw(RenderTarget& target, RenderState state = RenderState()) const override;

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
};

}

