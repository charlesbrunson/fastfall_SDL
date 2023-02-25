#include "fastfall/game/World.hpp"

#include "fastfall/game/WorldImGui.hpp"

#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/game/object/Object.hpp"

namespace ff {

World::World()
{
    WorldImGui::add(this);
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
    if (system<LevelSystem>().get_active(*this))
    {
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

        if (deltaTime > 0.0) {
            state.update_counter++;
        }
        state.update_time += deltaTime;
    }
}

void World::predraw(float interp, bool updated)
{
    if (auto* active = system<LevelSystem>().get_active(*this))
    {
        system<SceneSystem>().set_bg_color(active->getBGColor());
        system<SceneSystem>().set_size(active->size());
        system<ActorSystem>().predraw(*this, interp, updated);
        system<LevelSystem>().predraw(*this, interp, updated);
        system<EmitterSystem>().predraw(*this, interp, updated);
        system<SceneSystem>().set_cam_pos(system<CameraSystem>().getPosition(interp));
        system<SceneSystem>().predraw(*this, interp, updated);
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


std::optional<ID_ptr<Object>> World::create_object_from_data(ObjectLevelData& data) {
    auto id = create_entity();
    state._entities.at(id).actor = components<Actor>().peek_next_id();

    ActorInit init {
        .world      = *this,
        .entity_id  = id,
        .actor_id   = components<Actor>().peek_next_id(),
        .type       = ActorType::Actor,
        .priority   = ActorPriority::Normal
    };

    auto actor_id = components<Actor>().emplace(ObjectFactory::createFromData(init, data));
    if (auto* ptr = get(actor_id); ptr && ptr->initialized) {
        system_notify_created<Actor>(actor_id);
        auto obj_id = id_cast<Object>(actor_id);
        return ID_ptr<Object>{obj_id, get(obj_id) };
    }
    else {
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
    return state._comp_to_ent.at(id);
}

bool World::entity_has_actor(ID<Entity> id) const {
    return state._entities.at(id).actor.has_value();
}

void World::tie_component_entity(ComponentID cmp, ID<Entity> ent) {
    state._entities.at(ent).components.emplace(cmp);
    state._comp_to_ent.emplace(cmp, ent);
}

void World::untie_component_entity(ComponentID cmp, ID<Entity> ent) {
    state._entities.at(ent).components.erase(cmp);
    state._comp_to_ent.erase(cmp);
}

}