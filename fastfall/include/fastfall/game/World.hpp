#pragma once

#include "fastfall/util/id.hpp"
#include "fastfall/util/id_map.hpp"
#include "fastfall/game/ComponentID.hpp"
#include "fastfall/game/Entity.hpp"
#include "fastfall/game/object/Object.hpp"

#include "fastfall/engine/input/InputState.hpp"

#include "fastfall/game/WorldConfigComponents.hpp"
#include "fastfall/game/WorldConfigSystems.hpp"

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
        std::map<ComponentID, ID<Entity>> _comp_to_ent;

        // components
        Components::MapTuple _components;
        std::vector<ID<Drawable>>   erase_drawables_deferred;

        // systems
        Systems::Tuple _systems;

        // misc
        size_t update_counter = 0;
        secs   update_time = 0.0;

        // input
        InputState _input;
        InputSourceNull input_null;
    } state;

private:
    template<class T>
    constexpr auto& components() const {
        return const_cast<World&>(*this).components<T>();
    }

    template<class T>
    constexpr auto& components()
    {
        // figure out what container fits T
        constexpr size_t index = []<size_t... Ndx>(std::index_sequence<Ndx...>) constexpr {
            std::optional<size_t> opt_ndx;
            constexpr auto container_matches = []<size_t N>(std::optional<size_t>& opt_ndx, std::integral_constant<size_t, N>) constexpr {
                using Container = std::tuple_element_t<N, Components::MapTuple>;
                using Item      = typename Container::base_type;
                if (!opt_ndx && Container::template fits<T>()) {
                    opt_ndx = N;
                }
                return opt_ndx.has_value();
            };
            (container_matches(opt_ndx, std::integral_constant<size_t, Ndx>{}) || ...);
            return *opt_ndx;
        }(std::make_index_sequence<Components::Count>{});

        return std::get<index>(state._components);
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
    T& at(ID<T> id) { return components<T>().at(id); }

    template<class T>
    const T& at(ID<T> id) const { return components<T>().at(id); }

    template<class... Ts>
    requires (sizeof...(Ts) > 1)
    auto at(ID<Ts>... ids) { return std::forward_as_tuple(at(ids)...); }

    template<class... Ts>
    requires (sizeof...(Ts) > 1)
    auto at(ID<Ts>... ids) const { return std::forward_as_tuple(at(ids)...); }

    template<class T>
    T* get(ID<T> id) { return components<T>().get(id); }

    template<class T>
    const T* get(ID<T> id) const { return components<T>().get(id); }

    template<class T>
    T* get(std::optional<ID<T>> id) { return id ? components<T>().get(*id) : nullptr; }

    template<class T>
    const T* get(std::optional<ID<T>> id) const { return id ? components<T>().get(*id) : nullptr; }

    template<class... Ts>
    requires (sizeof...(Ts) > 1)
    auto get(ID<Ts>... ids) { return std::forward_as_tuple(get(ids)...); }

    template<class... Ts>
    requires (sizeof...(Ts) > 1)
    auto get(ID<Ts>... ids) const { return std::forward_as_tuple(get(ids)...); }

    const auto& entities() const { return state._entities; }

    // create entity
    ID<Entity> create_entity();

    template<class T_Actor, class... Args>
    requires valid_actor_ctor<T_Actor, Args...>
    std::optional<ID_ptr<T_Actor>> create_actor(Args&&... args) {
        auto id = create_entity();
        if (create_actor<T_Actor>(id, std::forward<Args>(args)...)) {
            auto actor_id = id_cast<T_Actor>(*state._entities.at(id).actor);
            system_notify_created<Actor>(actor_id);
            return ID_ptr<T_Actor>{actor_id, get(actor_id) };
        }
        else {
            erase(id);
            LOG_ERR_("failed to initialize entity");
        }
        return std::nullopt;
    }

    std::optional<ID_ptr<Object>> create_object_from_data(ObjectLevelData& data);

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
        auto& list = components<T>();
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
    inline auto& all() { return components<T>(); }
    template<class T>
    inline const auto& all() const { return components<T>(); }

	// access system
    template<class T>
    inline constexpr T& system() { return std::get<T>(state._systems); }
    template<class T>
    inline constexpr const T& system() const { return std::get<T>(state._systems); }

    // entity helpers
    const std::set<ComponentID>& components_of(ID<Entity> id) const;
    ID<Entity> entity_of(ComponentID id) const;

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
    requires std::constructible_from<T_Actor, ActorInit, Args...>
    bool create_actor(ID<Entity> id, Args&&... args) {
        auto& ent = state._entities.at(id);
        auto actor_id = components<Actor>().emplace(copyable_unique_ptr<Actor>());

        ActorInit init {
            .world       = *this,
            .entity_id   = id,
            .actor_id    = actor_id,
            .type        = ActorType::Actor,
            .priority    = ActorPriority::Normal,
        };

        if constexpr (valid_object<T_Actor>) {
            init.type     = ActorType::Object;
            init.priority = T_Actor::Type.priority;
        }

        ent.actor = actor_id;
        components<Actor>().emplace_at<T_Actor>(actor_id, init, std::forward<Args>(args)...);
        auto& actor = at(actor_id);
        return actor.is_initialized();
    }

    template<typename T>
    void system_notify_created(ID<T> t_id) {
        std::apply([&, this](auto&... system) {
            ([&, this]<class System>(System& sys){
                if constexpr (requires(System s, ID<T> i, World& w) { s.notify_created(w, i); }) {
                    sys.notify_created(*this, t_id);
                }
            }(system), ...);
        }, state._systems);
    }

    template<typename T>
    void system_notify_erased(ID<T> t_id) {
        std::apply([&, this](auto&... system) {
            ([&, this]<class System>(System& sys){
                if constexpr (requires(System s, ID<T> i, World& w) { s.notify_erased(w, i); }) {
                    sys.notify_erased(*this, t_id);
                }
            }(system), ...);
        }, state._systems);
    }

    void tie_component_entity(ComponentID cmp, ID<Entity> ent);
    void untie_component_entity(ComponentID cmp, ID<Entity> ent);

    void clean_drawables();
};

}

