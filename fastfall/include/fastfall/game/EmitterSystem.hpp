#pragma once

#include "fastfall/game/particle/Emitter.hpp"

namespace ff {

class World;

class EmitterSystem {
public:
    void update(World& world, secs deltaTime);
    void predraw(World& world, float interp, bool updated);

    void notify_created(World &world, ID<Emitter> id);
    void notify_erased(World &world, ID<Emitter> id);

private:

};

}