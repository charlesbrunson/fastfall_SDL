#pragma once

#include "fastfall/game/particle/Particle.hpp"

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/math.hpp"

#include <concepts>
#include <random>

namespace ff {

    template<class T>
    struct range {
        T min;
        T max;

        T pick(std::default_random_engine& engine)
        {
            std::uniform_int_distribution<> rdist{};
            if (max != min) {
                // [0.0, 1.0]
                double roll = (double)(rdist(engine)) / rdist.max();
                return min + (roll * (max - min));
            }
            else {
                return min;
            }
        }
    };

    class Emitter;

    // describe how particles should be created
    struct EmitterStrategy {
        range<secs> emit_rate = {0.1, 1.0}; // number of emissions per second
        range<unsigned> emit_count = {1, 1};
        secs max_lifetime = std::numeric_limits<secs>::max();

        long int max_particles = 100;

        Angle direction;
        float open_angle_degrees;
        range<float> velocity_range;

        bool inherits_vel;


        using ParticleTransformFn = std::function<void(const Emitter&, Particle&, secs)>;
        ParticleTransformFn particle_transform;

        using EmitterTransformFn = std::function<void(Emitter&, secs)>;
        EmitterTransformFn emitter_transform;

        Particle spawn(Vec2f emitter_pos, Vec2f emitter_vel, std::default_random_engine& rand);
    };

    struct EmitterListener {
        virtual void notify_particles_destroy(size_t begin, size_t count) = 0;
        virtual void notify_particles_pushed(size_t begin, size_t count) = 0;
    };

    class Emitter {
    public:
        Vec2f position;
        Vec2f velocity;
        bool is_enabled = true;
        EmitterStrategy strategy;

        std::vector<Particle>  particles;
        EmitterListener* listener = nullptr;

        void update(secs deltaTime);
        void clear_particles();
        void seed(size_t s);

        void set_strategy(EmitterStrategy strat);
        void reset_strategy();
        void backup_strategy();

        secs get_lifetime() const { return lifetime; };

    private:
        std::default_random_engine rand{};
        secs buffer = 0.0;
        secs lifetime = 0.0;
        size_t emit_count = 0;
        EmitterStrategy strategy_backup;

        void update_particle(Particle& p, secs deltaTime);
        void update_particles(secs deltaTime);
        void destroy_dead_particles();
        void spawn_particles(secs deltaTime);
    };
}
