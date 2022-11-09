#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/util/log.hpp"
#include <algorithm>
#include <execution>

namespace ff {

template<class T>
T pick_random(T min, T max, std::default_random_engine& engine)
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

Particle EmitterStrategy::spawn(Vec2f emitter_pos, Vec2f emitter_vel, std::default_random_engine& rand) const
{
    Particle p;
    float dist      = pick_random(0.f, scatter_max_radius, rand);
    float dist_ang  = pick_random(0.f, (float)M_PI * 2.f, rand);
    p.position = emitter_pos;
    p.position.x += local_spawn_area.left + pick_random(0.f, local_spawn_area.width, rand);
    p.position.y += local_spawn_area.top  + pick_random(0.f, local_spawn_area.height, rand);
    p.position += math::rotate(Vec2f{dist, 0.f}, dist_ang); //maVec2f{cosf(dist_ang), sinf(dist_ang)} * dist;
    p.prev_position = p.position;

    Vec2f vel = Vec2f{pick_random(particle_speed_min, particle_speed_max, rand), 0.f};
    vel = math::rotate(vel, direction);

    auto ang_offset = pick_random(-open_angle_degrees * 0.5f, open_angle_degrees * 0.5f, rand);
    vel = math::rotate(vel, Angle::Degree(ang_offset));

    p.velocity = vel + (inherits_vel ? emitter_vel : Vec2f{});
    return p;
}

Emitter::Emitter()
    : varr(ff::Primitive::POINT)
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

void Emitter::predraw(float interp)
{
    if (varr.size() < particles.capacity()) {
        varr = VertexArray{Primitive::POINT, particles.capacity()};
    }
    size_t ndx = 0;
    for (auto& p : particles) {
        varr[ndx].pos = p.prev_position + (p.position - p.prev_position) * interp;
        varr[ndx].color = Color::Red;
        ++ndx;
    }
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
        buffer += pick_random(strategy.emit_rate_min, strategy.emit_rate_max, rand);
        size_t init_size = particles.size();
        size_t created = 0;
        unsigned emit_count = pick_random(strategy.emit_count_min, strategy.emit_count_max, rand);
        for(auto i = emit_count; i > 0; --i)
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
    glPointSize(4.f);
    target.draw(varr);
    glPointSize(1.f);
}

}