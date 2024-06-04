#pragma once

#include "fastfall/game/particle/Emitter.hpp"

namespace ff {

class World;

class EmitterSystem {
public:
    void update(World& world, secs deltaTime);
    void predraw(World& world, predraw_state_t predraw_state);

    void notify_created(World &world, ID<Emitter> id);
    void notify_erased(World &world, ID<Emitter> id);
private:
    std::vector<ParticleEvent> events;
    std::vector<int>           events_per_emitter;
};

}