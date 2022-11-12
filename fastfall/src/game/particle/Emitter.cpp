#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/game/World.hpp"
#include "fastfall/resource/Resources.hpp"

#include <algorithm>
#include <execution>
#include <cmath>

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
    p.position += math::rotate(Vec2f{dist, 0.f}, dist_ang);
    p.prev_position = p.position;

    Vec2f vel = Vec2f{pick_random(particle_speed_min, particle_speed_max, rand), 0.f};
    vel = math::rotate(vel, direction);

    auto ang_offset = pick_random(-open_angle_degrees * 0.5f, open_angle_degrees * 0.5f, rand);
    vel = math::rotate(vel, Angle::Degree(ang_offset));

    p.velocity = vel + (inherits_vel ? emitter_vel : Vec2f{});
    return p;
}

Emitter::Emitter(EmitterStrategy str)
    : strategy(str)
{
}

void Emitter::update(World& w, secs deltaTime) {
    prev_position = position;
    lifetime += deltaTime;
    if (strategy.emitter_transform)
        strategy.emitter_transform(*this, deltaTime);

    update_particles(deltaTime);
    destroy_dead_particles();
    spawn_particles(deltaTime);
}

void Emitter::predraw(World& world, float interp, bool updated)
{
    auto& varr = world.at(varr_id);
    if (varr.size() < particles.size() * 6) {
        //varr = VertexArray{Primitive::POINT, particles.capacity()};
        size_t add_count = (particles.size() * 6) - varr.size();
        varr.insert(varr.size(), add_count, {});
    }

    auto* anim = Resources::get_animation(strategy.animation);
    auto& cfg = world.scene().config(varr_id);
    if (anim) {
        cfg.texture = anim->get_sprite_texture();
        auto invSize = cfg.texture->get()->inverseSize();

        Vec2f spr_size = Vec2f{ anim->area.getSize() } * 0.5f;

        float int_part;
        Vec2f inter_pos = prev_position + (position - prev_position) * interp;
        Vec2f subpixel = { std::modf(inter_pos.x, &int_part), std::modf(inter_pos.y, &int_part) };

        assert(varr.size() >= particles.size() * 6);
        size_t ndx = 0;
        for (auto& p : particles) {
            Vec2f center = p.prev_position + (p.position - p.prev_position) * interp;

            // nearest pixel
            center.x = floorf(center.x + 0.25f);
            center.y = floorf(center.y + 0.25f);
            center += subpixel;

            varr[ndx + 0].pos = center + Vec2f{ -spr_size.x, -spr_size.y };
            varr[ndx + 1].pos = center + Vec2f{  spr_size.x, -spr_size.y };
            varr[ndx + 2].pos = center + Vec2f{ -spr_size.x,  spr_size.y };

            varr[ndx + 3].pos = center + Vec2f{ -spr_size.x,  spr_size.y };
            varr[ndx + 4].pos = center + Vec2f{  spr_size.x, -spr_size.y };
            varr[ndx + 5].pos = center + Vec2f{  spr_size.x,  spr_size.y };

            size_t frame = floor(anim->framerateMS.size() * (float)(p.lifetime / strategy.max_lifetime));

            auto area = anim->area;
            area.left += frame * area.width;

            auto points = Rectf{ area }.toPoints();
            constexpr float tex_offset = 1.f / 16384.f;
            varr[ndx + 0].tex_pos = (points[0] + glm::vec2{ tex_offset,  tex_offset}) * invSize;
            varr[ndx + 1].tex_pos = (points[1] + glm::vec2{-tex_offset,  tex_offset}) * invSize;
            varr[ndx + 2].tex_pos = (points[2] + glm::vec2{ tex_offset, -tex_offset}) * invSize;

            varr[ndx + 3].tex_pos = (points[2] + glm::vec2{ tex_offset, -tex_offset}) * invSize;
            varr[ndx + 4].tex_pos = (points[1] + glm::vec2{-tex_offset,  tex_offset}) * invSize;
            varr[ndx + 5].tex_pos = (points[3] + glm::vec2{-tex_offset, -tex_offset}) * invSize;

            for(size_t n = 0; n < 6; ++n) {
                varr[ndx + n].color = Color::White;
            }
            ndx += 6;
        }
        while (ndx < varr.size()) {
            varr[ndx] = {};
            ++ndx;
        }
    }
    else {
        cfg.texture.reset();
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
    /*
    if (listener && it != particles.end())
    {
        listener->notify_particles_destroy(
                std::distance(particles.begin(), it),
                std::distance(it, particles.end()));
    }
    */
    particles.erase(it, particles.end());
}

void Emitter::spawn_particles(secs deltaTime) {
    buffer -= deltaTime;
    while (buffer < 0.0)
    {
        secs time = 1.f / pick_random(strategy.emit_rate_min, strategy.emit_rate_max, rand);
        buffer += time;
        size_t init_size = particles.size();
        size_t created = 0;
        unsigned emit_count = pick_random(strategy.emit_count_min, strategy.emit_count_max, rand);
        for(auto i = emit_count; i > 0; --i)
        {
            if (strategy.max_particles < 0
                || particles.size() < strategy.max_particles)
            {
                auto p = strategy.spawn(position, velocity, rand);
                // "catch up" the particle so stream is smoother
                p = update_particle(*this, p,  -buffer);

                if (p.is_alive) {
                    p.id = emit_count;
                    particles.push_back(p);
                    ++created;
                    ++emit_count;
                }
            }
        }

        /*
        if (listener && created) {
            listener->notify_particles_pushed(init_size, created);
        }
        */
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

/*
void Emitter::draw(RenderTarget& target, RenderState states) const {
    if (states.texture.exists()) {
        states.texture = texture;
    }

    target.draw(varr, states);
}
*/

}