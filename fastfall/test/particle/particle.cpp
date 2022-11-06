#include "gtest/gtest.h"

#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/util/log.hpp"

#include "ParticleRenderer.hpp"

using namespace ff;

constexpr secs one_tick = (1.0 / 60.0);

void print_particles(const Emitter& e) {
    size_t p_count = 0;
    LOG_INFO("---------------");
    for (auto& p : e.particles) {
        LOG_INFO("{:3}: t:{:2.3} p:{:2.3} v:{:2.3} m:{:2.3}", p_count, p.lifetime / one_tick, p.position, p.velocity, p.velocity.magnitude());
        ++p_count;
    }
}

TEST(particle, emitter)
{
    Emitter emit;
    emit.seed(0);
    emit.strategy.direction = Angle::Degree(-90);
    emit.strategy.open_angle_degrees = 360.f;
    emit.strategy.velocity_range = {200.f, 300.f};
    emit.strategy.emit_rate = {one_tick / 50, one_tick / 50};
    emit.strategy.max_particles = -1;
    emit.strategy.max_lifetime = one_tick * 120;

    /*
    emit.strategy.particle_transform = [](const Emitter& e, Particle& p, secs delta) {
        constexpr Vec2f gravity{0.f, 200.f};
        p.velocity += gravity * delta;
        p.velocity.x = math::reduce(p.velocity.x, abs(p.velocity.x) * 2.f * (float)delta, 0.f);
    };
    */

    emit.strategy.emitter_transform = [](Emitter& e, secs delta) {
        //if (e.strategy.open_angle_degrees < 360) {
        //    e.strategy.open_angle_degrees += 300.f * (float)delta;
        //}
        //auto prev = e.position;
        //e.position.x -= 100.f * (float)sin(delta);
        //e.velocity = (e.position - prev) / delta;

        secs life = e.get_lifetime();
        float radius = 60.f;
        Vec2f gp1 = Vec2f{0.f, 0.f};
        gp1.x += radius * (float)cos(life);
        gp1.y += radius * (float)sin(life);

        Vec2f gp2 = Vec2f{0.f, 0.f};
        gp2.x -= radius * (float)cos(life);
        gp2.y -= radius * (float)sin(life);

        e.strategy.particle_transform = [g1 = gp1, g2 = gp2](const Emitter& e, Particle& p, secs delta) {
            {
                Vec2f diff = g1 - p.position;
                float dist2 = diff.magnitudeSquared() / 2.f;
                p.velocity += (diff.unit() * 1000000.f / dist2) * delta;
            }

            {
                Vec2f diff = g2 - p.position;
                float dist2 = diff.magnitudeSquared() / 2.f;
                p.velocity += (diff.unit() * 1000000.f / dist2) * delta;
            }

            if (p.velocity.magnitude() > 300.f)
                p.velocity = p.velocity.unit() * 300.f;

        };
    };
    emit.backup_strategy();

    ParticleRenderer render{emit, {-100, -100, 200, 200}};
    render.draw();


    auto update = [&](unsigned count) {
        for (int i = 0; i < count; ++i) {
            emit.update(one_tick);
            render.draw();
        }
    };

    update(60 * 10);

    /*
    update(20);
    EXPECT_EQ(emit.particles.size(), 200);

    // max particles
    update(60);
    EXPECT_EQ(emit.particles.size(), 400);

    // let particles lifetime expire
    emit.strategy.max_particles = 0;
    update(40);
    EXPECT_EQ(emit.particles.size(), 200);

    update(40);
    EXPECT_EQ(emit.particles.size(), 0);
    */
}