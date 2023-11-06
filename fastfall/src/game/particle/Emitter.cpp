#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/game/World.hpp"
#include "fastfall/game/phys/ColliderRegion.hpp"
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

void Emitter::update_bounds() {
    particle_bounds = Rectf{ position.x, position.y, 0, 0 };
    for (auto& p : particles) {
        particle_bounds = math::rect_bound(particle_bounds, math::line_bounds( Linef{ p.prev_position, p.position } ));
    }
}

void Emitter::update(secs deltaTime) {
    if (deltaTime > 0.0) {
        lifetime += deltaTime;
        if (strategy.emitter_transform)
            strategy.emitter_transform(*this, deltaTime);

        destroy_dead_particles();
        update_particles(deltaTime);
        spawn_particles(deltaTime);
        update_bounds();

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

void Emitter::update_particle(const Emitter& e, Particle& p, secs deltaTime) {
    if (p.is_alive) {
        p.lifetime += deltaTime;

        if (e.strategy.particle_transform)
            e.strategy.particle_transform(e, p, deltaTime);

        p.prev_position = p.position;
        p.position += p.velocity * deltaTime;
    }
}

void Emitter::update_particles(secs deltaTime)
{
#if __cpp_lib_parallel_algorithm
    if (parallelize) {
        std::for_each(
                std::execution::par,
                particles.begin(),
                particles.end(),
                [this, &deltaTime](Particle& p){ update_particle(*this, p, deltaTime); }
                );
    }
    else {
        for (auto &p: particles) {
            update_particle(*this, p, deltaTime);
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
                update_particle(*this, p,  deltaTime + buffer);

                if (p.is_alive) {
                    // p.id = emit_count;
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

bool collide_surface(const ColliderRegion& region, const ColliderSurface* surf, Particle& p) {
    if (!surf) return false;

    Linef movement = { p.prev_position + region.getDeltaPosition(), p.position };
    Linef surface  = math::shift(surf->surface, region.getPosition());
    Vec2f normal   = math::vector(surface).lefthand().unit();

    if (math::dot(math::vector(movement), normal) < 0.f)
    {
        Vec2f intersect = math::intersection(movement, surface);
        Rectf bounds = math::rect_bound(math::line_bounds(movement), math::line_bounds(surface));
        if (bounds.contains(intersect)
            && math::line_has_point(movement, intersect, 0.01f)
            && math::line_has_point(surface,  intersect, 0.01f))
        {
            p.position = intersect;
            p.velocity = math::projection(region.velocity, normal, true)
                         + math::projection(p.velocity, normal.righthand(), true);
            return true;
        }
    }
    return false;
}

bool collide_quad(const ColliderRegion& region, const ColliderQuad& quad, Particle& p) {
    return collide_surface(region, quad.getSurface(Cardinal::N), p)
           || collide_surface(region, quad.getSurface(Cardinal::E), p)
           || collide_surface(region, quad.getSurface(Cardinal::S), p)
           || collide_surface(region, quad.getSurface(Cardinal::W), p);
}

void Emitter::apply_collision(const poly_id_map<ColliderRegion>& colliders) {
    for (const auto [rid, region] : colliders) {
        auto quad_area = region->in_rect(get_particle_bounds());

        if (debug_draw::hasTypeEnabled(debug_draw::Type::EMITTER)) {
            auto it = quad_area.begin();
            Rectf r_bounds = math::shift(Rectf{ it.get_tile_area() } * TILESIZE, region->getPosition());

            auto& p_bounds = createDebugDrawable<VertexArray, debug_draw::Type::EMITTER>(Primitive::LINE_LOOP, 4);

            for (int i = 0; i < p_bounds.size(); i++) {
                p_bounds[i].color = Color::White;
            }
            p_bounds[0].pos = math::rect_topleft(r_bounds);
            p_bounds[1].pos = math::rect_topright(r_bounds);
            p_bounds[2].pos = math::rect_botright(r_bounds);
            p_bounds[3].pos = math::rect_botleft(r_bounds);
        }

        for (const auto& quad : quad_area) {
            if (!quad.hasAnySurface() /* || quad.hasOneWay */ )
                continue;

            auto bounds = quad.get_bounds();

            if (!bounds)
                continue;

            *bounds = math::shift(*bounds, region->getPosition());

            if (debug_draw::hasTypeEnabled(debug_draw::Type::EMITTER)) {
                auto& q_bounds = createDebugDrawable<VertexArray, debug_draw::Type::EMITTER>(Primitive::LINE_LOOP, 4);

                for (int i = 0; i < q_bounds.size(); i++) {
                    q_bounds[i].color = Color::Green;
                }
                q_bounds[0].pos = math::rect_topleft(*bounds);
                q_bounds[1].pos = math::rect_topright(*bounds);
                q_bounds[2].pos = math::rect_botright(*bounds);
                q_bounds[3].pos = math::rect_botleft(*bounds);
            }

#if __cpp_lib_parallel_algorithm
            std::for_each(std::execution::par, particles.begin(), particles.end(),
                  [&](Particle& p){
                      collide_quad(*region, quad, p);
                  }
            );
#else
            for (auto &p: particles) {
                collide_quad(*region, quad, p);
            }
#endif
        }
    }
}

void imgui_component(World& w, ID<Emitter> id) {
    // TODO
}

}