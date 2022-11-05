#pragma once

#include "fastfall/game/particle/Particle.hpp"

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/math.hpp"

#include <concepts>
#include <random>

namespace ff {

    template<std::totally_ordered T>
    struct range {
        T min;
        T max;

        T pick(std::default_random_engine& engine)
        {
            std::uniform_int_distribution<> rdist{};
            if (max != min) {
                return min + (T(rdist(engine)) / T(rdist.max()) / (max - min));
            }
            else {
                return min;
            }
        }
    };

    struct EmitStrategy {
        range<secs> emit_rate = {0.1, 1.0}; // number of emissions per second
        range<unsigned> emit_count = {1, 1};
        secs max_lifetime = std::numeric_limits<secs>::max();

        unsigned max_particles = 100;

        Angle direction;
        Angle open_angle;
        range<float> velocity_range;

        bool inherits_vel;

        Particle spawn(Vec2f emitter_pos, Vec2f emitter_vel, std::default_random_engine& rand) {
            Particle p;
            p.position      = emitter_pos;
            p.prev_position = emitter_pos;

            Angle out = range<Angle>{
                direction - (open_angle / 2.f),
                direction + (open_angle / 2.f)
            }.pick(rand);

            Vec2f v_off = math::rotate(
                    Vec2f{velocity_range.pick(rand), 0.f},
                    out);

            p.velocity = (inherits_vel ? emitter_vel : Vec2f{}) + v_off;
            return p;
        }
    };

    class Emitter {
    public:
        Vec2f position;
        Vec2f velocity;
        EmitStrategy strat;

        void update(secs deltaTime)
        {
            // update particles
            for (auto& p : particles)
            {
                p.prev_position = p.position;
                p.position += p.velocity * deltaTime;
                p.lifetime += deltaTime;
            }

            // kill particles
            std::erase_if(particles, [this](Particle& p) {
                return p.lifetime >= strat.max_lifetime;
            });

            // spawn particles
            buffer -= deltaTime;
            while (buffer <= 0.0)
            {
                buffer += strat.emit_rate.pick(rand);
                unsigned count = strat.emit_count.pick(rand);
                for (unsigned i = 0; i < count; ++i)
                {
                    if (particles.size() < strat.max_particles) {
                        particles.push_back(strat.spawn(position, velocity, rand));
                    }
                }
            }
        }

        void clear() {
            particles.clear();
            buffer = 0.0;
        }

        void seed(size_t s) {
            rand.seed(s);
        }

        std::vector<Particle> particles;
    private:
        std::default_random_engine rand{};
        secs buffer = 0.0;
    };
}
