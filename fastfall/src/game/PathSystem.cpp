#include "fastfall/game/PathSystem.hpp"

#include "fastfall/game/World.hpp"


namespace ff {

void PathSystem::update(World& world, secs deltaTime) {
    for (auto [id, pm] : world.all<PathMover>()) {
        pm.update(world.at(pm.get_attach_id()), deltaTime);
    }
}

void PathSystem::notify_created(World& world, ID<PathMover> id) {
}

void PathSystem::notify_erased(World& world, ID<PathMover> id) {
}

}
