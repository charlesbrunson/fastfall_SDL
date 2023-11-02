#include "fastfall/game/World.hpp"

#include "fastfall/game/WorldImGui.hpp"

#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/user_types.hpp"
#include "fastfall/engine/audio.hpp"

//#include "fastfall/game/actor/Actor.hpp"

namespace ff {

World::World()
{
    WorldImGui::add(this);
    state._input.set_source(nullptr);
    state._comp_to_ent.reserve(128);
    system<AudioSystem>().set_destination_bus(&audio::primary_bus());
}

World::World(const World& other)
{
    WorldImGui::add(this);
    state = other.state;
    system<SceneSystem>().reset_proxy_ptrs(components<Drawable>());
}

World::World(World&& other) noexcept
{
    WorldImGui::add(this);
    state = std::move(other.state);
}

World& World::operator=(const World& other) {
    WorldImGui::add(this);
    state = other.state;
    system<SceneSystem>().reset_proxy_ptrs(components<Drawable>());
    return *this;
}

World& World::operator=(World&& other) noexcept {
    WorldImGui::add(this);
    state = std::move(other.state);
    return *this;
}

World::~World() {
    WorldImGui::remove(this);
}

void World::update(secs deltaTime) {
    if (deltaTime > 0.0 && system<LevelSystem>().get_active(*this))
    {
        //LOG_INFO("{:.25f}", deltaTime);
        system<SceneSystem>().update(*this, deltaTime);
        system<AttachSystem>().update(*this, deltaTime);
        state._input.update(deltaTime);

        system<LevelSystem>().update(*this,deltaTime);
        system<ActorSystem>().update(*this,deltaTime);

        system<TriggerSystem>().update(*this,deltaTime);
        system<PathSystem>().update(*this,deltaTime);
        system<AttachSystem>().update_attachpoints(*this, deltaTime, AttachPoint::Schedule::PostUpdate);

        system<CollisionSystem>().update(*this,deltaTime);
        system<AttachSystem>().update_attachpoints(*this, deltaTime, AttachPoint::Schedule::PostCollision);

        system<CameraSystem>().update(*this, deltaTime);
        system<EmitterSystem>().update(*this, deltaTime);
        system<ParticleCollisionSystem>().update(*this, deltaTime);

        system<AudioSystem>().update(deltaTime);

        for (auto [did, drawable] : all<Drawable>()) {
            drawable->update(deltaTime);
        }

        state.update_counter++;
        state.update_time += deltaTime;
    }
}

void World::predraw(predraw_state_t predraw_state)
{
    if (auto* active = system<LevelSystem>().get_active(*this))
    {
        system<SceneSystem>().set_bg_color(active->getBGColor());
        system<SceneSystem>().set_size(active->size());
        system<ActorSystem>().predraw(*this, predraw_state);
        system<LevelSystem>().predraw(*this, predraw_state);
        system<EmitterSystem>().predraw(*this, predraw_state);
        system<SceneSystem>().set_cam_pos(system<CameraSystem>().getPosition(predraw_state.interp));
        system<SceneSystem>().predraw(*this, predraw_state);
    }
    else
    {
        system<SceneSystem>().set_bg_color(ff::Color::Transparent);
    }
    clean_drawables();
}

void World::draw(RenderTarget& target, RenderState t_state) const
{
    system<SceneSystem>().draw(*this, target, t_state);
}

ID<Entity> World::create_entity() {
    return state._entities.create();
}


std::optional<ID_ptr<Actor>> World::create_actor_from_data(LevelObjectData& data) {
    auto id = create_entity();
    auto actor_id = components<Actor>().emplace(copyable_unique_ptr<Actor>());
    state._entities.at(id).actor = actor_id;

    const ActorType* type = ff::user_types::get_actor_type(data.typehash);
    if (!type) {
        LOG_WARN("Unknown actor type {}, hash {}", data.type, data.typehash);
        return std::nullopt;
    }

    ActorInit init {
        .world        = *this,
        .entity_id    = id,
        .actor_id     = actor_id,
        .type         = type,
        .level_object = &data
    };

    components<Actor>().emplace_at(actor_id, init.create());
    if (auto *ptr = get(actor_id); ptr && ptr->is_initialized()) {
        system_notify_created<Actor>(actor_id);
        return ID_ptr<Actor>{actor_id, get(actor_id)};
    } else {
        LOG_WARN("Failed to create actor type {}, hash {} from object", data.type, data.typehash);
        erase(id);
        return std::nullopt;
    }
}

void World::reset_entity(ID<Entity> id) {
    if (state._entities.exists(id)) {
        auto& ent = state._entities.at(id);
        auto actor = ent.actor;
        auto cmp_set = ent.components;
        if (actor) {
            system_notify_erased<Actor>(*actor);
            components<Actor>().erase(*actor);
            ent.actor.reset();
        }
        for (auto& c : cmp_set) {
            erase(c);
        }
    }
}

bool World::erase(ID<Entity> entity) {
    if (state._entities.exists(entity)) {
        reset_entity(entity);
        state._entities.erase(entity);
        return true;
    }
    return false;
}

bool World::erase(ComponentID component) {
    auto ent = entity_of(component);
    if (system<AttachSystem>().is_attached(component)) {
        system<AttachSystem>().erase(component);
    }
    std::visit([&, this]<typename T>(ID<T> id) {
            system_notify_erased(id);
            untie_component_entity(id, ent);
            if constexpr (std::same_as<T, Drawable>) {
                state.erase_drawables_deferred.insert(
                    std::lower_bound(
                            state.erase_drawables_deferred.begin(),
                            state.erase_drawables_deferred.end(),
                            id),
                    id);
            }
            else {
                components<T>().erase(id);
            }
        }, component);
    return true;
}

bool World::erase_all_components(ID<Entity> entity_id) {
    bool erased = true;
    auto components = components_of(entity_id);
    for (auto c : components) {
        erased &= erase(c);
    }
    return erased;
}

void World::clean_drawables() {
   for(auto id : state.erase_drawables_deferred) {
       components<Drawable>().erase(id);
   }
   state.erase_drawables_deferred.clear();
}
bool World::due_to_erase(ID<Drawable> id) const {
   return std::binary_search(state.erase_drawables_deferred.begin(), state.erase_drawables_deferred.end(), id);
}

const std::set<ComponentID>& World::components_of(ID<Entity> id) const {
    return state._entities.at(id).components;
}

ID<Entity> World::entity_of(ComponentID id) const {

    auto it = std::lower_bound(state._comp_to_ent.begin(), state._comp_to_ent.end(), state_t::comp_to_ent_t{ id, ID<Entity>{} });
    if (it != state._comp_to_ent.end() && it->c_id == id) {
        return it->e_id;
    }
    else {
        assert(false);
        return {};
    }
}

bool World::entity_has_actor(ID<Entity> id) const {
    return state._entities.at(id).actor.has_value();
}

void World::tie_component_entity(ComponentID cmp, ID<Entity> ent) {
    state._entities.at(ent).components.emplace(cmp);

    auto data = state_t::comp_to_ent_t{ cmp, ent };
    auto it = std::lower_bound(state._comp_to_ent.begin(), state._comp_to_ent.end(), data);
    if (it != state._comp_to_ent.end() && it->c_id == data.c_id) {
        it->e_id = ent;
    }
    else {
        state._comp_to_ent.insert(it, data);
    }
}

void World::untie_component_entity(ComponentID cmp, ID<Entity> ent) {
    state._entities.at(ent).components.erase(cmp);

    auto data = state_t::comp_to_ent_t{ cmp, ent };
    auto it = std::lower_bound(state._comp_to_ent.begin(), state._comp_to_ent.end(), data);
    if (it != state._comp_to_ent.end() && it->c_id == data.c_id) {
        state._comp_to_ent.erase(it);
    }
}

}