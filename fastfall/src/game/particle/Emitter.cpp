#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/util/log.hpp"

#include "fastfall/game/World.hpp"
#include "fastfall/game/phys/ColliderRegion.hpp"
#include "fastfall/resource/Resources.hpp"
#include "fastfall/render/DebugDraw.hpp"
#include "fastfall/util/enumerating_iterator.hpp"

#include <algorithm>

#include <cmath>
#include <ranges>


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

Particle EmitterStrategy::spawn(Vec2f emitter_pos, Vec2f emitter_vel, Angle emit_angle, std::default_random_engine& rand) const
{
    Particle p;
    float dist      = pick_random(0.f, scatter_max_radius, rand);
    float dist_ang  = pick_random(0.f, std::numbers::pi_v<float> * 2.f, rand);
    p.position = emitter_pos;
    p.position.x += local_spawn_area.left + pick_random(0.f, local_spawn_area.width, rand);
    p.position.y += local_spawn_area.top  + pick_random(0.f, local_spawn_area.height, rand);
    p.position += math::rotate(Vec2f{dist, 0.f}, dist_ang);
    p.prev_position = p.position;

    Vec2f vel = Vec2f{pick_random(particle_speed_min, particle_speed_max, rand), 0.f};
    vel = math::rotate(vel, emit_angle);

    auto ang_offset = pick_random(-open_angle_degrees * 0.5f, open_angle_degrees * 0.5f, rand);
    vel = math::rotate(vel, Angle::Degree(ang_offset));

    p.velocity = vel + inherits_vel * emitter_vel /* + inherits_acc * emitter_acc */;
    return p;
}

Emitter::Emitter(EmitterStrategy str)
    : strategy(str)
{
}

void Emitter::update_bounds() {
    particle_bounds = {};
    for (auto& p : particles) {
        [[likely]]
        if (particle_bounds) {
            particle_bounds = math::rect_bound(*particle_bounds, math::line_bounds( Linef{ p.prev_position, p.position } ));
        }
        else {
            particle_bounds = math::line_bounds( Linef{ p.prev_position, p.position } );
        }
    }
}

void Emitter::update(secs deltaTime, event_out_iter* events_out) {
    if (deltaTime > 0.0) {
        lifetime += deltaTime;
        if (strategy.emitter_transform)
            strategy.emitter_transform(*this, deltaTime);

        destroy_dead_particles(events_out);
        update_particles(deltaTime);
        spawn_particles(deltaTime);
        update_bounds();

        if (debug::enabled(debug::Emitter)) {

            if (particle_bounds) {
                auto p_bounds = debug::draw(
                        (const void *) this, Primitive::LINE_LOOP, 4);

                for (int i = 0; i < p_bounds.size(); i++) {
                    p_bounds[i].color = Color::Red;
                }
                p_bounds[0].pos = math::rect_topleft(*particle_bounds);
                p_bounds[1].pos = math::rect_topright(*particle_bounds);
                p_bounds[2].pos = math::rect_botright(*particle_bounds);
                p_bounds[3].pos = math::rect_botleft(*particle_bounds);
            }


            auto part_points = debug::draw(
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

        // TODO this really should just be a vert shader
        auto predraw_particle = [&](const auto& pair) {

            auto& p = pair.second;
            size_t ndx = pair.first * 6;

            // secs start_lifetime = p.lifetime - predraw_state.update_dt;
            secs exact_lifetime = p.lifetime + predraw_state.update_dt * (predraw_state.interp - 1.f);

            if (exact_lifetime < 0.0 || exact_lifetime >= strategy.max_lifetime)
            {
                varr[ndx + 0] = {};
                varr[ndx + 1] = {};
                varr[ndx + 2] = {};
                varr[ndx + 3] = {};
                varr[ndx + 4] = {};
                varr[ndx + 5] = {};
                return;
            }

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

            // triange 1
            varr[ndx + 0].tex_pos = (points[0] + glm::vec2{ tex_offset,  tex_offset}) * invSize;
            varr[ndx + 1].tex_pos = (points[1] + glm::vec2{-tex_offset,  tex_offset}) * invSize;
            varr[ndx + 2].tex_pos = (points[2] + glm::vec2{ tex_offset, -tex_offset}) * invSize;

            // triange 2
            varr[ndx + 3].tex_pos = (points[2] + glm::vec2{ tex_offset, -tex_offset}) * invSize;
            varr[ndx + 4].tex_pos = (points[1] + glm::vec2{-tex_offset,  tex_offset}) * invSize;
            varr[ndx + 5].tex_pos = (points[3] + glm::vec2{-tex_offset, -tex_offset}) * invSize;

            varr[ndx + 0].color = Color::White;
            varr[ndx + 1].color = Color::White;
            varr[ndx + 2].color = Color::White;
            varr[ndx + 3].color = Color::White;
            varr[ndx + 4].color = Color::White;
            varr[ndx + 5].color = Color::White;
        };

        if (strategy.draw_order == ParticleDrawOrder::NewestFirst) {
            std::for_each(
                make_enumerator(particles.cbegin()),
                make_enumerator(particles.cend()),
                predraw_particle);
        }
        else {
            std::for_each(
                make_enumerator(particles.crbegin()),
                make_enumerator(particles.crend()),
                predraw_particle);
        }

        for (size_t ndx = particles.size() * 6; ndx < varr.size(); ndx++) {
            varr[ndx] = {};
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

void Emitter::update_particle(const Emitter& e, Particle& p, secs deltaTime, bool born) {
    if (p.is_alive) {
        p.lifetime += deltaTime;

        if (e.strategy.particle_transform)
            e.strategy.particle_transform(e, p, deltaTime);

        p.prev_position = p.position;
        for (auto& acc : e.strategy.constant_accel) {
            p.velocity += acc * deltaTime;
            p.position += acc * deltaTime * deltaTime;
        }
        if (!born && e.strategy.move_with_emitter) {
            p.position += e.position - e.prev_position;
            // p.velocity += e.velocity - e.prev_velocity;
        }
        p.position += p.velocity * deltaTime;
        p.velocity *= (1.f - e.strategy.particle_damping);
    }
}

void Emitter::update_particles(secs deltaTime)
{
    std::for_each(
            particles.begin(),
            particles.end(),
            [this, &deltaTime](Particle& p) { update_particle(*this, p, deltaTime, false); }
            );
}

void Emitter::destroy_dead_particles(event_out_iter* events_out) {
    auto it = std::remove_if(particles.begin(), particles.end(), [&](Particle& p) {
        auto destroy = !p.is_alive || (strategy.max_lifetime >= 0 && p.lifetime >= strategy.max_lifetime);
        if (destroy && strategy.event_captures[ParticleEventType::Destroy]) {
            if (events_out)
                *events_out = { ParticleEventType::Destroy, p };
        }
        return destroy;
    });
    particles.erase(it, particles.end());
}



void Emitter::spawn_particles(secs deltaTime) {

    if ((strategy.emit_rate_min <= 0 && strategy.emit_rate_max <= 0)
        || !strategy.emission_enabled)
    {
        buffer = 0.0;
        return;
    }

    buffer -= deltaTime;

    while (buffer < 0.0)
    {
        float lerp = 1.f - ((deltaTime + buffer) / deltaTime);
        unsigned emit_count = pick_random(strategy.emit_count_min, strategy.emit_count_max, rand);
        while (emit_count-- > 0)
        {
            if (strategy.max_particles < 0
                || particles.size() < strategy.max_particles)
            {
                auto curr_pos = prev_position + ((position - prev_position) * lerp);
                auto curr_vel = prev_velocity + ((velocity - prev_velocity) * lerp);
                auto p = strategy.spawn(curr_pos, curr_vel, emit_angle, rand);
                update_particle(*this, p,  deltaTime + buffer, true);

                if (p.is_alive) {
                    p.id = total_emit_count++;
                    particles.push_back(p);
                }
            }
        }

        secs time = 1.0 / pick_random(strategy.emit_rate_min, strategy.emit_rate_max, rand);
        buffer += time;
    }
}

void Emitter::burst(Vec2f pos, Vec2f vel, Angle ang) {
    unsigned burst_count = pick_random(strategy.burst_count_min, strategy.burst_count_max, rand);
    for(auto i = 0; i < burst_count; ++i)
    {
        if (strategy.burst_chance > pick_random(0.f, 1.f, rand)) {
            if (strategy.max_particles < 0
                || particles.size() < strategy.max_particles) {
                auto p = strategy.spawn(pos, vel, ang, rand);
                if (p.is_alive) {
                    p.id = total_emit_count++;
                    particles.push_back(p);
                }
            }
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

bool collide_surface(const ColliderRegion& region, const ColliderSurface* surf, const EmitterStrategy& cfg, Particle& p) {
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

            std::default_random_engine rand{ p.id };

            auto bounce = -math::projection(p.velocity, normal, true) * pick_random(cfg.collision_bounce_min, cfg.collision_bounce_max, rand);
            auto scatter_ang = math::angle(normal) + Angle::Degree(pick_random(-cfg.collision_scatter_angle_max, cfg.collision_scatter_angle_max, rand));
            auto scatter = Vec2f{ cosf(scatter_ang.radians()), sinf(scatter_ang.radians()) } * pick_random(cfg.collision_scatter_force_min, cfg.collision_scatter_force_max, rand);

            p.velocity = math::projection(region.velocity, normal, true)
                    + math::projection(p.velocity, normal.righthand(), true) * (1.f - cfg.collision_damping)
                    + bounce
                    + scatter;

            p.collision_normal = normal;

            return true;
        }
    }
    return false;
}

bool collide_quad(const ColliderRegion& region, const ColliderQuad& quad, const EmitterStrategy& cfg, Particle& p) {
    return collide_surface(region, quad.getSurface(Cardinal::N), cfg, p)
        || collide_surface(region, quad.getSurface(Cardinal::E), cfg, p)
        || collide_surface(region, quad.getSurface(Cardinal::S), cfg, p)
        || collide_surface(region, quad.getSurface(Cardinal::W), cfg, p);
}

void Emitter::apply_collision(const poly_id_map<ColliderRegion>& colliders, event_out_iter* events_out) {

    if (!strategy.collision_enabled || !get_particle_bounds()) {
        /*
        std::for_each(
            std::execution::par,
            particles.begin(),
            particles.end(),
            [&](Particle& p) {
                p.collision_normal.reset();
            });
        */
        return;
    }


    for (const auto [rid, region] : colliders) {
        auto quad_area = region->in_rect(*get_particle_bounds());

        if (debug::enabled(debug::Emitter)) {
            auto it = quad_area.begin();
            Rectf r_bounds = math::shift(Rectf{ it.get_tile_area() } * TILESIZE, region->getPosition());

            auto p_bounds = debug::draw(Primitive::LINE_LOOP, 4);

            for (auto & p_bound : p_bounds) {
                p_bound.color = Color::White;
            }
            p_bounds[0].pos = math::rect_topleft(r_bounds);
            p_bounds[1].pos = math::rect_topright(r_bounds);
            p_bounds[2].pos = math::rect_botright(r_bounds);
            p_bounds[3].pos = math::rect_botleft(r_bounds);
        }

        for (auto quad : quad_area) {
            if (!quad->hasAnySurface() /* || quad.hasOneWay */ )
                continue;

            auto bounds = quad->get_bounds();

            if (!bounds)
                continue;

            *bounds = math::shift(*bounds, region->getPosition());

            if (debug::enabled(debug::Emitter)) {
                auto q_bounds = debug::draw(Primitive::LINE_LOOP, 4);

                for (auto & q_bound : q_bounds) {
                    q_bound.color = Color::Green;
                }
                q_bounds[0].pos = math::rect_topleft(*bounds);
                q_bounds[1].pos = math::rect_topright(*bounds);
                q_bounds[2].pos = math::rect_botright(*bounds);
                q_bounds[3].pos = math::rect_botleft(*bounds);
            }

            std::for_each(
                particles.begin(),
                particles.end(),
                [&](Particle& p) {
                    collide_quad(*region, *quad, strategy, p);
                    p.is_alive &= !(strategy.collision_destroys && p.collision_normal);
                }
            );

            if (strategy.event_captures[ParticleEventType::Collide]) {
                for (auto &p: particles) {
                    if (p.collision_normal) {
                        if (events_out)
                            *events_out = { ParticleEventType::Collide, p };

                        p.collision_normal.reset();
                    }
                }
            }
        }
    }
}

void imgui_component(World& w, ID<Emitter> id) {
    // TODO
    auto& cmp = w.at(id);
    auto& cfg = cmp.strategy;

    ImGui::SeparatorText("Emitter Configuration");
    ImGui::Checkbox("Emission Enabled", &cfg.emission_enabled);

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


    ImGui::DragFloat("Opening", &cfg.open_angle_degrees, 1, 0, 360);
    ImGui::DragFloatRange2("Particle speed", &cfg.particle_speed_min, &cfg.particle_speed_max);
    ImGui::DragFloat("Particle dampening", &cfg.particle_damping, 0.01f, -1.f, 1.f);
    ImGui::DragFloat4("Particle spawn area", &cfg.local_spawn_area.left);
    ImGui::DragFloat("Particle spawn radius", &cfg.scatter_max_radius, 0.1f, 0.f);
    ImGui::DragFloat("Inherit parent velocity", &cfg.inherits_vel, 0.01f, -1.f, 1.f);
    //ImGui::DragFloat("Inherit parent acceleration", &cfg.inherits_acc, 0.01f, -100.f, 100.f);
    ImGui::Checkbox("Move with emitter", &cfg.move_with_emitter);
    ImGui::Checkbox("Collision enabled", &cfg.collision_enabled);
    ImGui::Checkbox("Collision destroys", &cfg.collision_destroys);
    ImGui::DragFloatRange2("Collision bounce", &cfg.collision_bounce_min, &cfg.collision_bounce_max, 0.005f, 0.f, 1.f);
    ImGui::DragFloat("Collision damping", &cfg.collision_damping, 0.005f, 0.f, 1.f);
    ImGui::DragFloat("Collision scatter angle", &cfg.collision_scatter_angle_max, 1.f, 0.f, 90.f);
    ImGui::DragFloatRange2("Collision scatter force", &cfg.collision_scatter_force_min, &cfg.collision_scatter_force_max, 1.f, 0.f, FLT_MAX);

    ImGui::DragFloat2("Force 1", &cfg.constant_accel[0].x, 10.f);
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        cfg.constant_accel[0] = Vec2f{};
    }

    ImGui::DragFloat2("Force 2", &cfg.constant_accel[1].x, 10.f);
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        cfg.constant_accel[1] = Vec2f{};
    }

    ImGui::DragFloat2("Force 3", &cfg.constant_accel[2].x, 10.f);
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        cfg.constant_accel[2] = Vec2f{};
    }

    ImGui::DragFloat2("Force 4", &cfg.constant_accel[3].x, 10.f);
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        cfg.constant_accel[3] = Vec2f{};
    }


    ImGui::Text("Particle Draw Order: %s", (cfg.draw_order == ParticleDrawOrder::NewestFirst ? "Newest First" : "Oldest First" ));
    ImGui::SameLine();
    if (ImGui::SmallButton("Switch")) {
        cfg.draw_order = (cfg.draw_order == ParticleDrawOrder::NewestFirst ? ParticleDrawOrder::OldestFirst : ParticleDrawOrder::NewestFirst);
    }

    ImGui::SeparatorText("Emitter State");
    ImGui::Text("Particle Count: %zu", cmp.particles.size());
    float density = cmp.get_particle_bounds() ? (cmp.particles.size() / (cmp.get_particle_bounds()->getArea() / TILESIZE_F)) : -1.f;
    ImGui::Text("Particle Density: %f", density);

    float degrees = cmp.emit_angle.degrees();
    if (ImGui::DragFloat("Direction", &degrees, 1, -180, 181)) {
        cmp.emit_angle = Angle::Degree(degrees);
    }
}

}