#include "fastfall/game/systems/LevelSystem.hpp"

#include "fastfall/game/World.hpp"

namespace ff {

void LevelSystem::notify_created(World& world, ID<Level> id)
{
    if (!active_level) {
        active_level = id;
    }
}

void LevelSystem::notify_erased(World& world, ID<Level> id)
{
    if (active_level && *active_level == id) {
        active_level.reset();
    }
}

std::optional<ID<Level>> LevelSystem::get_active_id() const {
    return active_level;
}

void LevelSystem::update(World& world, secs deltaTime) {
    //TODO
    //for (auto [id, tl] : world.all<TileLayer>()) {
    //    tl.update(world, deltaTime);
    //}
}

void LevelSystem::predraw(World& world, predraw_state_t predraw_state) {
    //TODO
    //for (auto [id, tl] : world.all<TileLayer>()) {
    //    tl.predraw(world, interp, updated);
    //}
}

Level* LevelSystem::get_active(World& world) const {
    if (active_level) {
        return world.get(*active_level);
    }
    return nullptr;
}

void LevelSystem::set_active(ID<Level> id) {
    active_level = id;
}

}