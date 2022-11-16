#pragma once

#include "fastfall/engine/time/time.hpp"
#include "fastfall/game/path/PathMover.hpp"

namespace ff {

class World;

class PathSystem {
public:
    void update(World& world, secs deltaTime);
    void notify_created(World& world, ID<PathMover> id);
    void notify_erased(World& world, ID<PathMover> id);
};

}
