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
                double roll = (double)(rdist(engine) % 101) / 100.0;
                return min + (roll * (max - min));
            }
            else {
                return min;
            }
        }
    };

    // describe how particles should be created
    struct EmitterStrategy {
        range<secs> emit_rate = {0.1, 1.0}; // number of emissions per second
        range<unsigned> emit_count = {1, 1};
        secs max_lifetime = std::numeric_limits<secs>::max();

        unsigned max_particles = 100;

        Angle direction;
        Angle open_angle;
        range<float> velocity_range;

        bool inherits_vel;

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
        EmitterStrategy strategy;

        using Transform = std::function<void(const Emitter&, Particle&)>;

        void update(secs deltaTime);
        void clear();
        void seed(size_t s);

        std::vector<Particle>  particles;
        std::vector<Transform> transforms;
        EmitterListener* listener = nullptr;

    private:
        std::default_random_engine rand{};
        secs buffer = 0.0;

        void update_particle(Particle& p, secs deltaTime);
        void update_particles(secs deltaTime);
        void destroy_dead_particles();
        void spawn_particles(secs deltaTime);
    };
}
