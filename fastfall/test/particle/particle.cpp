#include "gtest/gtest.h"

#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/util/log.hpp"

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
    emit.strategy.direction = Angle::Degree(0);
    emit.strategy.open_angle = Angle::Degree(90);
    emit.strategy.velocity_range = {10.f, 10.f};
    emit.strategy.emit_rate = {one_tick, one_tick};
    emit.strategy.max_particles = 20;
    emit.strategy.max_lifetime = one_tick * 40;

    // emit rate is 1 per tick
    for (int i = 0; i < 10; ++i)
        emit.update(one_tick);

    EXPECT_EQ(emit.particles.size(), 10);
    //print_particles(emit);

    // max particles
    for (int i = 0; i < 30; ++i)
        emit.update(one_tick);

    EXPECT_EQ(emit.particles.size(), 20);
    //print_particles(emit);

    // let particles lifetime expire
    emit.strategy.max_particles = 0;

    for (int i = 0; i < 10; ++i)
        emit.update(one_tick);

    EXPECT_EQ(emit.particles.size(), 10);
    //print_particles(emit);

    for (int i = 0; i < 10; ++i)
        emit.update(one_tick);

    EXPECT_EQ(emit.particles.size(), 0);
    //print_particles(emit);

}