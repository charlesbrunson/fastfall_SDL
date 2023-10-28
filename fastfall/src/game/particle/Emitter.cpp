#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/game/World.hpp"
#include "fastfall/resource/Resources.hpp"
#include "fastfall/render/DebugDraw.hpp"

#include <algorithm>

#if __cpp_lib_parallel_algorithm
#include <execution>
#endif

#include <cmath>



namespace ff {

template<class T>
T pick_random(T min, T max, std::default_random_engine& engine)
{
    std::uniform_int_distribution<> rdist{};
    if (max != min) {
        // [0.0, 1.0]
        double roll = (double)(rdist(engine)) / (rdist.max)();
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

void Emitter::update(secs deltaTime) {
    if (deltaTime > 0.0) {
        lifetime += deltaTime;
        if (strategy.emitter_transform)
            strategy.emitter_transform(*this, deltaTime);

        destroy_dead_particles();
        update_particles(deltaTime);
        spawn_particles(deltaTime);

        particle_bounds = Rectf{ position.x, position.y, 0, 0 };
        for (auto& p : particles) {
            particle_bounds = math::rect_bound(particle_bounds, Rectf{ p.position.x, p.position.y, 0, 0 });
        }

        if (debug_draw::hasTypeEnabled(debug_draw::Type::EMITTER)) {

            auto& p_bounds = createDebugDrawable<VertexArray, debug_draw::Type::EMITTER>(
                    (const void*)this, Primitive::LINE_LOOP, 4);

            for (int i = 0; i < p_bounds.size(); i++) {
                p_bounds[i].color = Color::Red;
            }
            p_bounds[0].pos = math::rect_topleft(particle_bounds);
            p_bounds[1].pos = math::rect_topright(particle_bounds);
            p_bounds[2].pos = math::rect_botright(particle_bounds);
            p_bounds[3].pos = math::rect_botleft(particle_bounds);


            auto& part_points = createDebugDrawable<VertexArray, debug_draw::Type::EMITTER>(
                    (const void*)this, Primitive::LINES, particles.size() * 4);

            size_t ndx = 0;
            for (auto& p : particles) {
                part_points[ndx + 0].color = Color::Red;
                part_points[ndx + 1].color = Color::Red;
                part_points[ndx + 2].color = Color::Red;
                part_points[ndx + 3].color = Color::Red;

                part_points[ndx + 0].pos = p.position + Vec2f{ -1.f,  0.f };
                part_points[ndx + 1].pos = p.position + Vec2f{  1.f,  0.f };
                part_points[ndx + 2].pos = p.position + Vec2f{  0.f, -1.f };
                part_points[ndx + 3].pos = p.position + Vec2f{  0.f,  1.f };

                ndx += 4;
            }
        }
    }
}

void Emitter::predraw(VertexArray& varr, SceneConfig& cfg, predraw_state_t predraw_state)
{
    if (varr.size() < particles.size() * 6) {
        size_t add_count = (particles.size() * 6) - varr.size();
        varr.insert(varr.size(), add_count, {});
    }

    auto* anim = AnimDB::get_animation(strategy.animation);
    if (anim) {
        cfg.rstate.texture = anim->get_sprite_texture();
        auto invSize = cfg.rstate.texture.get()->inverseSize();

        Vec2f spr_size = Vec2f{ anim->area.getSize() } * 0.5f;

        Vec2f inter_pos = prev_position + (position - prev_position) * predraw_state.interp;

        Vec2f subpixel = {
            floorf(inter_pos.x + 0.5f) - inter_pos.x,
            floorf(inter_pos.y + 0.5f) - inter_pos.y,
        };

        assert(varr.size() >= particles.size() * 6);
        size_t ndx = 0;
        for (auto& p : particles) {

            secs start_lifetime = p.lifetime - predraw_state.update_dt;
            secs exact_lifetime = p.lifetime - predraw_state.update_dt * (1.f - predraw_state.interp);
            secs end_lifetime   = p.lifetime;


            if (exact_lifetime < 0.0 || exact_lifetime >= strategy.max_lifetime)
            {
                for (size_t n = 0; n < 6; ++n) {
                    varr[ndx + n] = {};
                }
                ndx += 6;
                continue;
            }


            bool born = start_lifetime < 0.f;
            //bool dies = end_lifetime >= strategy.max_lifetime;

            float p_interp = predraw_state.interp;
            if (born) {
                p_interp = (p_interp * end_lifetime) - start_lifetime;
                //LOG_INFO("BORN: {} -> {}", predraw_state.interp, p_interp);
            }
            /*
            if (dies) {
                p_interp = p_interp - (end_lifetime - strategy.max_lifetime) / predraw_state.update_dt;
                LOG_INFO("DIES: {} -> {}", predraw_state.interp, p_interp);
            }
            */

            Vec2f center = p.prev_position + (p.position - p.prev_position) * predraw_state.interp;

            // snap to pixel offset of emitter
            center.x = floorf(center.x + 0.5f);
            center.y = floorf(center.y + 0.5f);
            center -= subpixel;

            varr[ndx + 0].pos = center + Vec2f{ -spr_size.x, -spr_size.y };
            varr[ndx + 1].pos = center + Vec2f{  spr_size.x, -spr_size.y };
            varr[ndx + 2].pos = center + Vec2f{ -spr_size.x,  spr_size.y };

            varr[ndx + 3].pos = center + Vec2f{ -spr_size.x,  spr_size.y };
            varr[ndx + 4].pos = center + Vec2f{  spr_size.x, -spr_size.y };
            varr[ndx + 5].pos = center + Vec2f{  spr_size.x,  spr_size.y };

            size_t frame = floor((float)(anim->framerateMS.size()) * (float)(exact_lifetime / strategy.max_lifetime));

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

            for (size_t n = 0; n < 6; ++n) {
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
        cfg.rstate.texture = Texture::getNullTexture();
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
        p.lifetime += deltaTime;

        if (e.strategy.particle_transform)
            e.strategy.particle_transform(e, p, deltaTime);

        p.prev_position = p.position;
        p.position += p.velocity * deltaTime;
    }
    return p;
}

void Emitter::update_particles(secs deltaTime)
{
#if __cpp_lib_parallel_algorithm
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
#else
    for (auto &p: particles) {
        p = update_particle(*this, p, deltaTime);
    }
#endif
}

void Emitter::destroy_dead_particles() {
    auto it = std::remove_if(particles.begin(), particles.end(), [this](Particle& p) {
        return !p.is_alive || (strategy.max_lifetime >= 0 && p.lifetime >= strategy.max_lifetime);
    });
    particles.erase(it, particles.end());
}

void Emitter::spawn_particles(secs deltaTime) {

    buffer -= deltaTime;
    while (buffer < 0.0)
    {
        size_t created = 0;
        unsigned emit_count = pick_random(strategy.emit_count_min, strategy.emit_count_max, rand);
        for(auto i = emit_count; i > 0; --i)
        {
            if (strategy.max_particles < 0
                || particles.size() < strategy.max_particles)
            {
                auto p = strategy.spawn(position, velocity, rand);
                // "catch up" the particle so stream is smoother
                p = update_particle(*this, p,  deltaTime + buffer);

                if (p.is_alive) {
                    p.id = emit_count;
                    particles.push_back(p);
                    ++created;
                    ++emit_count;
                }
            }
        }

        secs time = 1.f / pick_random(strategy.emit_rate_min, strategy.emit_rate_max, rand);
        buffer += time;
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

void imgui_component(World& w, ID<Emitter> id) {
    // TODO
}

}