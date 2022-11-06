#include "fastfall/game/particle/Emitter.hpp"

namespace ff {

Particle EmitterStrategy::spawn(Vec2f emitter_pos, Vec2f emitter_vel, std::default_random_engine& rand) {
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

void Emitter::update(secs deltaTime) {
    update_particles(deltaTime);
    destroy_dead_particles();
    spawn_particles(deltaTime);
}

void Emitter::clear() {
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

}