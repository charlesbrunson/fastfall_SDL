#pragma once

#include "fastfall/game/particle/Particle.hpp"

#include "fastfall/engine/time/time.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/render/util/Texture.hpp"
#include "fastfall/render/drawable/VertexArray.hpp"
#include "fastfall/resource/asset/AnimAssetTypes.hpp"
#include "fastfall/resource/asset/AnimAsset.hpp"
#include "fastfall/util/id.hpp"
#include "fastfall/util/id_map.hpp"

#include <concepts>
#include <random>

namespace ff {

    class Emitter;
    class SceneConfig;
    class ColliderRegion;

    // describe how particles should be created/handled
    struct EmitterStrategy {

        // number of emissions per second
        secs emit_rate_min = 10;
        secs emit_rate_max = 10;

        // number of particles per emission
        unsigned emit_count_min = 1;
        unsigned emit_count_max = 1;

        // max lifetime of each particle, <0 is infinite
        secs max_lifetime = 10.0;

        // max particles at any time, <0 is infinite
        long int max_particles = 10;

        // direction of the particles
        Angle direction = Angle::Radian(0.f);

        // variation of direction, in degrees
        float open_angle_degrees = 0.f;

        // speed of the particles
        float particle_speed_min = 100.f;
        float particle_speed_max = 100.f;

        // factor applied to the velocity per second
        float particle_damping = 0.f;

        // local rect the particles are spawned in
        Rectf local_spawn_area = {};

        // max distance to scatter particles, radially (note: combine with local_spawn_area for a bezelled area)
        float scatter_max_radius = 0.f;

        // particles inherit velocity of emitter
        bool inherits_vel = false;

        // collision
        bool has_collision = false;

        // if it has collision, bounce factor
        float collision_bounce = 0.f;

        // animation for particles to play
        AnimIDRef animation;

        // function applied to each particle each tick
        using ParticleTransformFn = std::function<void(const Emitter&, Particle&, secs)>;
        ParticleTransformFn particle_transform;

        // function applied to the emitter each tick
        using EmitterTransformFn = std::function<void(Emitter&, secs)>;
        EmitterTransformFn emitter_transform;

        Particle spawn(Vec2f emitter_pos, Vec2f emitter_vel, std::default_random_engine& rand) const;
    };

    class Emitter {
    public:
        Emitter() = default;
        Emitter(EmitterStrategy str);

        Vec2f position;
        Vec2f prev_position;
        Vec2f velocity;
        bool is_enabled = true;
        bool parallelize = true;
        EmitterStrategy strategy;

        std::vector<Particle>  particles;

        void update(secs deltaTime);
        void predraw(VertexArray& varr, SceneConfig& cfg, predraw_state_t predraw_state);

        void clear_particles();
        void reset(size_t s) {
            lifetime = 0.0;
            total_emit_count = 0;
            reset_strategy();
            clear_particles();
            seed(s);
        };
        void seed(size_t s);

        void set_strategy(EmitterStrategy strat);
        void reset_strategy();
        void backup_strategy();

        secs get_lifetime() const { return lifetime; };

        void apply_collision(const poly_id_map<ColliderRegion>& colliders);

        void set_drawid(ID<VertexArray> id) { varr_id = id; }
        ID<VertexArray> get_drawid() const { return varr_id; }

        Rectf get_particle_bounds() const { return particle_bounds; }

    private:
        AnimIDRef curr_anim;
        const Animation* animation = nullptr;

        std::default_random_engine rand{};
        secs buffer = 0.0;
        secs lifetime = 0.0;
        size_t total_emit_count = 0;
        EmitterStrategy strategy_backup;
        ID<VertexArray> varr_id;

        Rectf particle_bounds;

        static void update_particle(const Emitter& e, Particle& p, secs deltaTime);
        void update_particles(secs deltaTime);
        void destroy_dead_particles();
        void spawn_particles(secs deltaTime);
        void update_bounds();
    };

    class World;
    void imgui_component(World& w, ID<Emitter> id);

}
