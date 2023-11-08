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
        p.velocity *= (1.f - e.strategy.particle_damping);
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

        secs time = 1.0 / pick_random(strategy.emit_rate_min, strategy.emit_rate_max, rand);
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

bool collide_surface(const ColliderRegion& region, const ColliderSurface* surf, Particle& p, float collision_bounce, float collision_damping) {
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

            auto bounce = (collision_bounce > 0 ? -math::projection(p.velocity, normal, true) * collision_bounce : Vec2f{});

            p.velocity = math::projection(region.velocity, normal, true)
                    + math::projection(p.velocity * (1.f - collision_damping), normal.righthand(), true)
                    + bounce;

            return true;
        }
    }
    return false;
}

bool collide_quad(const ColliderRegion& region, const ColliderQuad& quad, Particle& p, float collision_bounce, float collision_damping) {
    return collide_surface(region, quad.getSurface(Cardinal::N), p, collision_bounce, collision_damping)
        || collide_surface(region, quad.getSurface(Cardinal::E), p, collision_bounce, collision_damping)
        || collide_surface(region, quad.getSurface(Cardinal::S), p, collision_bounce, collision_damping)
        || collide_surface(region, quad.getSurface(Cardinal::W), p, collision_bounce, collision_damping);
}

void Emitter::apply_collision(const poly_id_map<ColliderRegion>& colliders) {
    if (!strategy.has_collision)
        return;

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
                  [&](Particle& p) {
                      collide_quad(*region, quad, p, strategy.collision_bounce, strategy.collision_damping);
                  }
            );
#else
            for (auto &p: particles) {
                collide_quad(*region, quad, p, strategy.collision_bounce, strategy.collision_damping);
            }
#endif
        }
    }
}

void imgui_component(World& w, ID<Emitter> id) {
    // TODO
    auto& cmp = w.at(id);
    auto& cfg = cmp.strategy;

    if (ImGui::CollapsingHeader("Emitter Configuration", ImGuiTreeNodeFlags_DefaultOpen)) {

        float emit_min = static_cast<float>(cfg.emit_rate_min);
        float emit_max = static_cast<float>(cfg.emit_rate_max);
        if (ImGui::DragFloatRange2("Emit Rate", &emit_min, &emit_max, 0.5f, 0.f, 10000.f)) {
            cfg.emit_rate_min = static_cast<secs>(emit_min);
            cfg.emit_rate_max = static_cast<secs>(emit_max);
        }

        int count_min = static_cast<int>(cfg.emit_count_min);
        int count_max = static_cast<int>(cfg.emit_count_max);
        if (ImGui::DragIntRange2("Emit Count", &count_min, &count_max, 1, 0, 20)) {
            cfg.emit_count_min = static_cast<unsigned>(count_min);
            cfg.emit_count_max = static_cast<unsigned>(count_max);
        }

        constexpr static secs lifetime_max_min = 0.0;
        constexpr static secs lifetime_max_max = 10.0;
        ImGui::DragScalar("Max lifetime", ImGuiDataType_Double, (void*)&cfg.max_lifetime, 0.01f, (void*)&lifetime_max_min, (void*)&lifetime_max_max);

        ImGui::DragInt("Max particles", &cfg.max_particles, 1, -1, 1000);

        float degrees = cfg.direction.degrees();
        if (ImGui::DragFloat("Direction", &degrees, 1, -180, 180)) {
            cfg.direction = Angle::Degree(degrees);
        }

        ImGui::DragFloat("Opening", &cfg.open_angle_degrees, 1, 0, 180);
        ImGui::DragFloatRange2("Particle speed", &cfg.particle_speed_min, &cfg.particle_speed_max);
        ImGui::DragFloat("Particle dampening", &cfg.particle_damping, 0.01f, -1.f, 1.f);
        ImGui::DragFloat4("Particle spawn area", &cfg.local_spawn_area.left);
        ImGui::DragFloat("Particle spawn radius", &cfg.scatter_max_radius, 0.1f, 0.f);
        ImGui::Checkbox("Inherit parent velocity", &cfg.inherits_vel);
        ImGui::Checkbox("Has collision", &cfg.has_collision);
        ImGui::DragFloat("Collision bounce", &cfg.collision_bounce, 0.01f, 0.f, 1.f);
        ImGui::DragFloat("Collision damping", &cfg.collision_damping, 0.01f, 0.f, 1.f);
    }


}

}