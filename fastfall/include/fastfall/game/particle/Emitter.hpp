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
    class World;

    enum class ParticleDrawOrder {
        NewestFirst,
        OldestFirst,
    };

    enum class ParticleEventType : uint8_t {
        Destroy = 0,
        Collide = 1
    };

    struct ParticleEvent {
        ParticleEventType type;
        Particle particle;
    };

    // describe how particles should be created/handled
    struct EmitterStrategy {

        bool emission_enabled = true;

        // number of emissions per second
        secs emit_rate_min = 10;
        secs emit_rate_max = 10;

        // number of particles per emission
        unsigned emit_count_min = 1;
        unsigned emit_count_max = 1;

        // number of particles per burst
        unsigned burst_count_min = 1;
        unsigned burst_count_max = 1;

        float burst_chance = 1.f;

        // max lifetime of each particle, <0 is infinite
        secs max_lifetime = 10.0;

        // max particles at any time, <0 is infinite
        int max_particles = 10;

        // direction of the particles
        // Angle direction = Angle::Radian(0.f);

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

        // particles inherit part of velocity of the emitter
        float inherits_vel = 0.f;
        //float inherits_acc = 0.f;

        // add delta position of emitter each tick
        bool move_with_emitter = false;

        // collision
        bool collision_enabled = false;

        // if it has collision, bounce factor
        float collision_bounce_min = 0.f;
        float collision_bounce_max = 0.f;
        // random force applied after a collision
        float collision_scatter_force_min = 0.f;
        float collision_scatter_force_max = 0.f;

        float collision_scatter_angle_max = 0.f;
        // dampening applied after collisions
        float collision_damping = 0.f;
        // particle is destroyed after any collision
        bool collision_destroys = false;

        ParticleDrawOrder draw_order = ParticleDrawOrder::NewestFirst;

        std::array<Vec2f, 4> constant_accel = {};

        // animation for particles to play
        AnimIDRef animation;

        // function applied to each particle each tick
        using ParticleTransformFn = std::function<void(const Emitter&, Particle&, secs)>;
        ParticleTransformFn particle_transform;

        // function applied to the emitter each tick
        using EmitterTransformFn = std::function<void(Emitter&, secs)>;
        EmitterTransformFn emitter_transform;

        struct event_captures_t {
            event_captures_t() = default;

            template<typename ...E>
            requires(sizeof...(E) > 0 && (std::same_as<E, ParticleEventType> && ...))
            event_captures_t(E... event_types) {
                (set(event_types), ...);
            }

            uint8_t event_types = 0;

            inline bool operator[](ParticleEventType type) const {
                return (event_types & (1 << static_cast<uint8_t>(type))) > 1;
            }

            void set(ParticleEventType type) {
                event_types |= (1 << static_cast<uint8_t>(type));
            }
            void unset(ParticleEventType type) {
                event_types &= ~(1 << static_cast<uint8_t>(type));
            }

        } event_captures;

        // function applied to the emitter each tick
        using EventsCallbackFn = std::function<void(World& w, std::span<const ParticleEvent> events)>;
        EventsCallbackFn events_callback;

        Particle spawn(Vec2f emitter_pos, Vec2f emitter_vel, Angle emit_angle, std::default_random_engine& rand) const;
    };

    class Emitter {
    public:
        using event_out_iter = std::back_insert_iterator<std::vector<ParticleEvent>>;

        Emitter() = default;
        explicit Emitter(EmitterStrategy str);

        Angle emit_angle;

        Vec2f position;
        Vec2f prev_position;
        Vec2f velocity;
        Vec2f prev_velocity;
        bool is_enabled = true;

        // bool parallelize = true;
        EmitterStrategy strategy;

        std::vector<Particle> particles;

        void update(secs deltaTime, event_out_iter* events_out = nullptr);
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

        void burst(Vec2f pos, Vec2f vel, Angle ang);

        void set_strategy(EmitterStrategy strat);
        void reset_strategy();
        void backup_strategy();

        secs get_lifetime() const { return lifetime; };

        void apply_collision(const poly_id_map<ColliderRegion>& colliders, event_out_iter* events_out = nullptr);

        void set_drawid(ID<VertexArray> id) { varr_id = id; }
        ID<VertexArray> get_drawid() const { return varr_id; }

        const std::optional<Rectf>& get_particle_bounds() const { return particle_bounds; }

    private:
        // AnimIDRef curr_anim;
        const Animation* animation = nullptr;

        std::default_random_engine rand = {};
        secs buffer = 0.0;
        secs lifetime = 0.0;
        unsigned total_emit_count = 0;
        EmitterStrategy strategy_backup;
        ID<VertexArray> varr_id;

        std::optional<Rectf> particle_bounds;

        static void update_particle(const Emitter& e, Particle& p, secs deltaTime, bool born);
        void update_particles(secs deltaTime);
        void destroy_dead_particles(event_out_iter* events_out = nullptr);
        void spawn_particles(secs deltaTime);
        void update_bounds();
    };

    class World;
    void imgui_component(World& w, ID<Emitter> id);

}
