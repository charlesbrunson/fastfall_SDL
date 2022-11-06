#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/util/log.hpp"

namespace ff {

Particle EmitterStrategy::spawn(Vec2f emitter_pos, Vec2f emitter_vel, std::default_random_engine& rand) {
    Particle p;
    p.position      = emitter_pos;
    p.prev_position = emitter_pos;

    Vec2f v_off = Vec2f{velocity_range.pick(rand), 0.f};
    v_off = math::rotate(v_off, direction);

    Angle ang_offset = Angle::Degree(
        range<float>{
            -open_angle_degrees,
            open_angle_degrees
        }.pick(rand) / 2.f
    );
    v_off = math::rotate(v_off, ang_offset);

    p.velocity = (inherits_vel ? emitter_vel : Vec2f{}) + v_off;
    return p;
}

void Emitter::update(secs deltaTime) {
    lifetime += deltaTime;
    if (strategy.emitter_transform)
        strategy.emitter_transform(*this, deltaTime);

    update_particles(deltaTime);
    destroy_dead_particles();
    spawn_particles(deltaTime);

}

void Emitter::clear_particles() {
    particles.clear();
    buffer = 0.0;
}

void Emitter::seed(size_t s) {
    rand.seed(s);
}

void Emitter::update_particle(Particle& p, secs deltaTime) {
    if (p.is_alive) {
        if (p.lifetime >= strategy.max_lifetime) {
            p.is_alive = false;
        } else {
            if (strategy.particle_transform)
                strategy.particle_transform(*this, p, deltaTime);

            p.prev_position = p.position;
            p.position += p.velocity * deltaTime;
            p.lifetime += deltaTime;
        }
    }
}

void Emitter::update_particles(secs deltaTime)
{
    for (auto& p : particles)
    {
        update_particle(p, deltaTime);
    }
}

void Emitter::destroy_dead_particles() {
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

void Emitter::spawn_particles(secs deltaTime) {
    buffer -= deltaTime;
    while (buffer < 0.0)
    {
        buffer += strategy.emit_rate.pick(rand);

        size_t init_size = particles.size();
        size_t created = 0;

        for(auto i = strategy.emit_count.pick(rand); i > 0; --i)
        {
            if (strategy.max_particles < 0
                || particles.size() < strategy.max_particles)
            {
                auto p = strategy.spawn(position, velocity, rand);
                update_particle(p, deltaTime);

                if (p.is_alive) {
                    p.id = emit_count;
                    particles.push_back(p);
                    ++created;
                    ++emit_count;
                }
            }
        }

        if (listener && created) {
            listener->notify_particles_pushed(init_size, created);
        }
    }
}

void Emitter::set_strategy(EmitterStrategy strat) {
    strategy = strat;
    strategy_backup = strat;
}

void Emitter::reset_strategy() {
    strategy = strategy_backup;
}

void Emitter::backup_strategy() {
    strategy_backup = strategy;
}

}