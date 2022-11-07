#pragma once

#include "fastfall/game/particle/Particle.hpp"

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/resource/asset/AnimAssetTypes.hpp"

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

        // number of emissions per second
        range<secs>     emit_rate           = {0.1, 1.0};
        // number of particles per emission
        range<unsigned> emit_count          = {1, 1};
        // max lifetime of each particle, <0 is infinite
        secs            max_lifetime        = -1;
        // max particles at any time, <0 is infinite
        long int        max_particles       = 10;
        // direction of the particles
        Angle           direction           = Angle::Radian(0.f);
        // variation of direction, in degrees
        float           open_angle_degrees  = 0.f;
        // speed of the particles
        range<float>    particle_speed      = { 0.f, 0.f };

        // local rect the particles are spawned in
        Rectf           local_spawn_area    = {};
        // max distance to scatter particles, radially (note: combine with local_spawn_area for a bezelled area)
        float           scatter_max_radius  = 0.f;

        // particles inherit velocity of emitter
        bool            inherits_vel        = false;

        // animation for particles to play
        AnimIDRef animation;

        // function applied to each particle each tick
        using ParticleTransformFn = std::function<void(const Emitter&, Particle&, secs)>;
        ParticleTransformFn particle_transform;

        // function applied to the emitter each tick
        using EmitterTransformFn = std::function<void(Emitter&, secs)>;
        EmitterTransformFn emitter_transform;

        Particle spawn(Vec2f emitter_pos, Vec2f emitter_vel, std::default_random_engine& rand);
    };

    struct EmitterListener {
        virtual void notify_particles_destroy(size_t begin, size_t count) = 0;
        virtual void notify_particles_pushed(size_t begin, size_t count) = 0;
    };

    class Emitter : public Drawable {
    public:
        Emitter();

        Vec2f position;
        Vec2f velocity;
        bool is_enabled = true;
        bool parallelize = true;
        EmitterStrategy strategy;

        std::vector<Particle>  particles;
        EmitterListener* listener = nullptr;

        void update(secs deltaTime);
        void predraw(float interp);

        void clear_particles();
        void reset(size_t s) {
            lifetime = 0.0;
            emit_count = 0;
            reset_strategy();
            clear_particles();
            seed(s);
        };
        void seed(size_t s);

        void set_strategy(EmitterStrategy strat);
        void reset_strategy();
        void backup_strategy();

        secs get_lifetime() const { return lifetime; };

    private:
        void draw(RenderTarget& target, RenderState states = RenderState{}) const override;

        AnimIDRef curr_anim;
        const Animation* animation = nullptr;

        std::default_random_engine rand{};
        secs buffer = 0.0;
        secs lifetime = 0.0;
        size_t emit_count = 0;
        EmitterStrategy strategy_backup;
        VertexArray vertex;

        static Particle update_particle(const Emitter& e, Particle p, secs deltaTime);
        void update_particles(secs deltaTime);
        void destroy_dead_particles();
        void spawn_particles(secs deltaTime);
    };

}
