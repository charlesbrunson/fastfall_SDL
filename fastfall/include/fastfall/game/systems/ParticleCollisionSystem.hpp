#pragma once

#include "fastfall/util/id.hpp"
#include "fastfall/engine/time/time.hpp"

#include "fastfall/game/particle/Emitter.hpp"

namespace ff {

class World;
//class ColliderQuad;

class ParticleCollisionSystem {
public:
    void update(World& world, secs deltaTime);

private:
    void collide_emitter_particles(World& world, Emitter& emitter);
    //void collide_particle(const ColliderQuad& quad, Particle& particle);
};


}