#include "gtest/gtest.h"

#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/util/log.hpp"

#include "ParticleRenderer.hpp"

using namespace ff;

constexpr secs one_tick = (1.0 / 50.0);

void print_particles(const Emitter& e) {
    size_t p_count = 0;
    LOG_INFO("---------------");
    for (auto& p : e.particles) {
        LOG_INFO("{:3}: t:{:2.3} p:{:2.3} v:{:2.3} m:{:2.3}", p_count, p.lifetime / one_tick, p.position, p.velocity, p.velocity.magnitude());
        ++p_count;
    }
}

Vec2f attract(float mag, Vec2f gpos, Vec2f ppos, secs delta) {
    Vec2f diff = gpos - ppos;
    float dist2 = diff.magnitudeSquared();
    return (diff.unit() * mag / dist2) * delta;
}

TEST(particle, emitter)
{

    Emitter emit;
    emit.seed(0);
    emit.strategy.direction = Angle::Degree(90);
    emit.strategy.open_angle_degrees = 0.f;
    emit.strategy.particle_speed = {300.f, 300.f};
    emit.strategy.scatter_max_radius = 0.f;
    emit.strategy.local_spawn_area = {-400, -410, 800, 10.f};
    emit.strategy.emit_rate = { one_tick / 50, one_tick / 50 };
    emit.strategy.max_particles = -1;
    emit.strategy.max_lifetime = -1;

    emit.strategy.emitter_transform = [](Emitter& e, secs delta) {
        if (e.get_lifetime() > 10.0) {
            e.strategy.max_particles = 0;
        }
    };

    emit.strategy.particle_transform = [](const Emitter& e, Particle& p, secs delta){
        constexpr Vec2f gpos = {0.f, 200.f};
        p.velocity += attract(40'000'000.f, gpos, p.position, delta);

        if (p.velocity.magnitude() > 1000.f)
            p.velocity = p.velocity.unit() * 1000.f;

        if ((gpos - p.position).magnitude() < 40.f)
            p.is_alive = false;
    };

    emit.backup_strategy();

    ParticleRenderer render{emit, {-400, -400, 800, 800}};

    auto update = [&](unsigned count) {
        for (int i = 0; i < count; ++i) {
            emit.update(one_tick);
            render.draw();
        }
    };

    update(600 / one_tick);

    /*
    std::locale::global(std::locale("es_CO.UTF-8"));
    auto timer = std::chrono::steady_clock{};
    emit.parallelize = false;

    // run for 20s
    auto start = timer.now();
    update(30 / one_tick);
    auto duration = timer.now() - start;

    LOG_INFO("DURATION UNPARALLEL = {:20L}us", std::chrono::duration_cast<std::chrono::microseconds>(duration).count())

    emit.reset(0);
    emit.parallelize = true;

    // run for 20s
    start = timer.now();
    update(30 / one_tick);
    duration = timer.now() - start;

    LOG_INFO("DURATION PARALLEL   = {:20L}us", std::chrono::duration_cast<std::chrono::microseconds>(duration).count())

    */

    /*
    Emitter emit;
    emit.seed(0);
    emit.strategy.direction = Angle::Degree(-90);
    emit.strategy.open_angle_degrees = 360.f;
    emit.strategy.velocity_range = {200.f, 300.f};
    emit.strategy.emit_rate = {one_tick / 50, one_tick / 50};
    emit.strategy.max_particles = -1;
    emit.strategy.max_lifetime = one_tick * 120;
    emit.strategy.emitter_transform = [](Emitter& e, secs delta) {
        secs life = e.get_lifetime();
        float radius = 60.f;
        Vec2f g1 = radius * Vec2f{cosf(life), sinf(life)};
        Vec2f g2 = -g1;

        e.strategy.particle_transform = [g1, g2](const Emitter& e, Particle& p, secs delta) {
            Vec2f g1diff = g1 - p.position;
            Vec2f g2diff = g2 - p.position;
            float g1dist2 = g1diff.magnitudeSquared();
            float g2dist2 = g2diff.magnitudeSquared();
            p.velocity += ((g1diff.unit() * 500000.f / g1dist2) * delta)
                        + ((g2diff.unit() * 500000.f / g2dist2) * delta);

            if (p.velocity.magnitude() > 300.f)
                p.velocity = p.velocity.unit() * 300.f;
        };
    };
    */

}