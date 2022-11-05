#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/engine/time/time.hpp"

namespace ff {

    // base particle class
    struct ParticleBase {
        Vec2f position;
        Vec2f prev_position;
        Vec2f velocity;
        //unsigned bounce_count = 0;
    };

    struct ParticleEmitter {

        void emit(secs deltaTime) {
            
        }

        void update_particle(ParticleBase& p, secs deltaTime) {
            p.prev_position = p.position;
            p.position += p.velocity * deltaTime;
        }

        Vec2f position;
        Vec2f velocity;

        float emit_rate = 1.f;
        unsigned curr_particle_max = 10;

        // particle count
        size_t total_particles = 0;
        size_t total_particles_max = 100;

        // lifetime
        //secs lifetime;
        //secs max_lifetime = std::numeric_limits<secs>::max();

    };





}