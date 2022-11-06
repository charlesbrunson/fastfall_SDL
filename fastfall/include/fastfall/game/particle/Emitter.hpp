#pragma once

#include "fastfall/game/particle/Particle.hpp"

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/util/log.hpp"

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

        Particle spawn(Vec2f emitter_pos, Vec2f emitter_vel, std::default_random_engine& rand) {
            Particle p;
            p.position      = emitter_pos;
            p.prev_position = emitter_pos;

            Angle out = Angle::Radian(range<float>{
                direction.radians() - (open_angle.radians() / 2.f),
                direction.radians() + (open_angle.radians() / 2.f)
            }.pick(rand));

            Vec2f v_off = math::rotate(
                    Vec2f{velocity_range.pick(rand), 0.f},
                    out);

            p.velocity = (inherits_vel ? emitter_vel : Vec2f{}) + v_off;
            return p;
        }
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

        void update(secs deltaTime) {
            update_particles(deltaTime);
            destroy_dead_particles();
            spawn_particles(deltaTime);
        }


        void clear() {
            particles.clear();
            buffer = 0.0;
        }

        void seed(size_t s) {
            rand.seed(s);
        }

        std::vector<Particle>  particles;
        std::vector<Transform> transforms;
        EmitterListener* listener = nullptr;

    private:
        std::default_random_engine rand{};
        secs buffer = 0.0;

        void update_particle(Particle& p, secs deltaTime) {
            if (p.is_alive) {
                if (p.lifetime >= strategy.max_lifetime) {
                    p.is_alive = false;
                } else {
                    for (auto &tf: transforms) {
                        tf(*this, p);
                    }
                    p.prev_position = p.position;
                    p.velocity += p.accel * deltaTime;
                    p.position += p.velocity * deltaTime;
                    p.lifetime += deltaTime;
                }
            }
        }

        void update_particles(secs deltaTime)
        {
            for (auto& p : particles)
            {
                update_particle(p, deltaTime);
            }
        }

        void destroy_dead_particles() {
            auto it = std::remove_if(particles.begin(), particles.end(), [](Particle& p) {
                return !p.is_alive;
            });
            if (listener && it != particles.end())
            {
                listener->notify_particles_destroy(
                        std::distance(particles.begin(), it),
                        std::distance(it, particles.end()));
            }
            particles.erase(it, particles.end());
        }

        void spawn_particles(secs deltaTime) {
            buffer -= deltaTime;
            while (buffer < 0.0)
            {
                buffer += strategy.emit_rate.pick(rand);

                size_t init_size = particles.size();
                size_t created = 0;

                for(auto i = strategy.emit_count.pick(rand); i > 0; --i)
                {
                    if (particles.size() < strategy.max_particles)
                    {
                        auto p = strategy.spawn(position, velocity, rand);
                        update_particle(p, deltaTime);

                        if (p.is_alive) {
                            particles.push_back(p);
                            ++created;
                        }
                    }
                }

                if (listener && created) {
                    listener->notify_particles_pushed(init_size, created);
                }
            }
        }
    };
}
