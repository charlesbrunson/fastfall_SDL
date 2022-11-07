#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/util/log.hpp"
#include <algorithm>
#include <execution>

namespace ff {

Particle EmitterStrategy::spawn(Vec2f emitter_pos, Vec2f emitter_vel, std::default_random_engine& rand) {
    Particle p;

    float dist      = range<float>{0.f, scatter_max_radius}.pick(rand);
    float dist_ang  = range<float>{0.f, M_PI * 2}.pick(rand);
    p.position = emitter_pos;
    p.position.x += local_spawn_area.left + range<float>{0.f, local_spawn_area.width}.pick(rand);
    p.position.y += local_spawn_area.top  + range<float>{0.f, local_spawn_area.height}.pick(rand);
    p.position += Vec2f{cosf(dist_ang), sinf(dist_ang)} * dist;

    p.prev_position = p.position;

    Vec2f v_off = Vec2f{particle_speed.pick(rand), 0.f};
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

Emitter::Emitter()
    : vertex(ff::Primitive::POINT)
{
}

void Emitter::update(secs deltaTime) {
    lifetime += deltaTime;
    if (strategy.emitter_transform)
        strategy.emitter_transform(*this, deltaTime);

    update_particles(deltaTime);
    destroy_dead_particles();
    spawn_particles(deltaTime);

}

void predraw(float interp) {

}

void Emitter::clear_particles() {
    particles.clear();
    buffer = 0.0;
}

void Emitter::seed(size_t s) {
    rand.seed(s);
}

Particle Emitter::update_particle(const Emitter& e, Particle p, secs deltaTime) {
    if (p.is_alive) {
        if (e.strategy.max_lifetime >= 0 && p.lifetime >= e.strategy.max_lifetime) {
            p.is_alive = false;
        } else {
            if (e.strategy.particle_transform)
                e.strategy.particle_transform(e, p, deltaTime);

            p.prev_position = p.position;
            p.position += p.velocity * deltaTime;
            p.lifetime += deltaTime;
        }
    }
    return p;
}

void Emitter::update_particles(secs deltaTime)
{
    if (parallelize) {
        std::transform(
                std::execution::par,
                particles.cbegin(),
                particles.cend(),
                particles.begin(),
                [this, &deltaTime](Particle p){ return update_particle(*this, p, deltaTime); });
    }
    else {
        for (auto &p: particles) {
            p = update_particle(*this, p, deltaTime);
        }
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
                update_particle(*this, p, deltaTime);

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

void Emitter::draw(RenderTarget& target, RenderState states) const {
    target.draw(vertex);
}

}