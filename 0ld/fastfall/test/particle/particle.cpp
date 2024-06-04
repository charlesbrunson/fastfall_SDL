#include "gtest/gtest.h"

#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/util/log.hpp"

#include "ParticleRenderer.hpp"

using namespace ff;

constexpr secs one_tick = (1.0 / 50.0);


auto get_emitter() {
    Emitter emit;
    emit.seed(0);
    emit.strategy.open_angle_degrees = 0.f;
    emit.strategy.particle_speed_min = 100.f;
    emit.strategy.particle_speed_max = 100.f;
    emit.strategy.scatter_max_radius = 0.f;
    emit.strategy.emit_rate_min = 50;
    emit.strategy.emit_rate_max = 50;
    emit.strategy.max_particles = 50;
    emit.strategy.max_lifetime = 2.0;

    emit.backup_strategy();
    return emit;
}

TEST(particle, emitter)
{
    auto emit = get_emitter();
    //ParticleRenderer render{emit, {-400, -400, 800, 800}};

    EXPECT_TRUE(emit.velocity == Vec2f{});
    EXPECT_TRUE(emit.position == Vec2f{});
    EXPECT_TRUE(emit.prev_position == Vec2f{});

    for (auto i{0}; i < 25; ++i) {
        emit.update(one_tick);
    }
    EXPECT_EQ(emit.particles.size(), 25);

    for (auto i{0}; i < 25; ++i) {
        emit.update(one_tick);
    }
    EXPECT_EQ(emit.particles.size(), 50);

    for (auto i{0}; i < 25; ++i) {
        emit.update(one_tick);
    }
    EXPECT_EQ(emit.particles.size(), 50);


    emit.strategy.max_particles = 0;
    for (auto i{0}; i < 49; ++i) {
        emit.update(one_tick);
    }
    emit.update(one_tick);
    EXPECT_EQ(emit.particles.size(), 25);

    for (auto i{0}; i < 50; ++i) {
        emit.update(one_tick);
    }
    EXPECT_EQ(emit.particles.size(), 0);

}