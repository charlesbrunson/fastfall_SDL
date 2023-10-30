
#include "fastfall/game/systems/ParticleCollisionSystem.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/render/DebugDraw.hpp"

#include <chrono>

#if __cpp_lib_parallel_algorithm
#include <execution>
#endif

namespace ff {

struct aabb_t {
    Cardinal dir;
    bool has_surface;
    Linef surface;
    Vec2f normal;

    float penetration = 0.f;
};

cardinal_array<aabb_t> do_aabb(const ColliderQuad& quad, Rectf quad_bounds, Vec2f position) {

    cardinal_array<aabb_t> AABB;

    for (auto dir : direction::cardinals) {
        AABB[dir].dir         = dir;
        AABB[dir].surface     = quad.surfaces[dir].collider.surface;
        AABB[dir].has_surface = quad.surfaces[dir].hasSurface;
        AABB[dir].normal      = math::vector(AABB[dir].surface).unit().lefthand();
    }

    // horizontal
    AABB[Cardinal::E].penetration = (quad_bounds.left + quad_bounds.width) - position.x;
    AABB[Cardinal::W].penetration = position.x - quad_bounds.left;

    // vertical
    Vec2f n_intersection = math::intersection(AABB[Cardinal::N].surface, Linef{position, position + Vec2f{ 0.f, 1.f }});
    Vec2f s_intersection = math::intersection(AABB[Cardinal::S].surface, Linef{position, position + Vec2f{ 0.f, 1.f }});

    AABB[Cardinal::N].normal = math::vector(AABB[Cardinal::N].surface).unit().lefthand();
    AABB[Cardinal::S].normal = math::vector(AABB[Cardinal::S].surface).unit().lefthand();

    if (n_intersection.y <= quad_bounds.top) {
        n_intersection.y = quad_bounds.top;
        AABB[Cardinal::N].normal = Vec2f{ 0.f, -1.f };
    }

    if (s_intersection.y >= quad_bounds.top + quad_bounds.height) {
        s_intersection.y = quad_bounds.top + quad_bounds.height;
        AABB[Cardinal::S].normal = Vec2f{ 0.f, 1.f };
    }

    AABB[Cardinal::N].penetration = position.y - n_intersection.y;
    AABB[Cardinal::S].penetration = s_intersection.y - position.y;

    return AABB;
}

void collide_particle(const ColliderRegion& region, const ColliderQuad& quad, Rectf quad_bounds, Particle& particle) {

    Vec2f rel_delta_pos = (particle.position - region.getPosition()) - (particle.prev_position - region.getPrevPosition());

    auto prev = do_aabb(quad, math::shift(quad_bounds, -region.getDeltaPosition()), particle.prev_position);
    auto curr = do_aabb(quad, quad_bounds, particle.position);

    static constexpr cardinal_array<std::pair<Cardinal, Cardinal>> splits = {
        std::pair<Cardinal, Cardinal>{ Cardinal::E, Cardinal::W }, // Cardinal::N
        std::pair<Cardinal, Cardinal>{ Cardinal::N, Cardinal::S }, // Cardinal::E
        std::pair<Cardinal, Cardinal>{ Cardinal::E, Cardinal::W }, // Cardinal::S
        std::pair<Cardinal, Cardinal>{ Cardinal::N, Cardinal::S }, // Cardinal::W
    };

    for (auto dir : direction::cardinals) {

        auto [side1, side2] = splits[dir];

        bool valid_axis = (prev[dir].has_surface && curr[dir].has_surface)
                && (prev[dir].penetration <= 0.f && curr[dir].penetration >= 0.f)
                && math::dot(rel_delta_pos, curr[dir].normal) <= 0
                && (curr[side1].penetration >= 0 && curr[side2].penetration >= 0);

        if (valid_axis) {
            auto& aabb = curr[dir];

            /*
            if (debug_draw::hasTypeEnabled(debug_draw::Type::EMITTER)) {
                auto& debug_surf = createDebugDrawable<VertexArray, debug_draw::Type::EMITTER>(Primitive::TRIANGLE_STRIP, 4);

                debug_surf[0].color = Color::Red;
                debug_surf[1].color = Color::Red;
                debug_surf[2].color = Color::Red;
                debug_surf[3].color = Color::Red;

                debug_surf[0].pos = aabb.surface.p1 + region.getPosition();
                debug_surf[1].pos = aabb.surface.p2 + region.getPosition();
                debug_surf[2].pos = aabb.surface.p1 + region.getPosition() - aabb.normal;
                debug_surf[3].pos = aabb.surface.p2 + region.getPosition() - aabb.normal;
            }
            */

            particle.position += aabb.normal * aabb.penetration;
            particle.velocity = math::projection(region.velocity, aabb.normal, true)
                              + math::projection(particle.velocity, aabb.normal.righthand(), true);
        }
    }
}

void ParticleCollisionSystem::update(World& world, secs deltaTime) {

    using namespace std::chrono;

    auto start = steady_clock::now();

    size_t particle_count = 0;


    for (auto [id, e] : world.all<Emitter>()) {
        if (e.strategy.has_collision) {
            collide_emitter_particles(world, e);
            particle_count += e.particles.size();
        }
    }

    auto end = steady_clock::now();

    duration<double, std::milli> elapsed = (end - start);
    LOG_INFO("\t{}ms - {} particles", elapsed.count(), particle_count);
}

void ParticleCollisionSystem::collide_emitter_particles(const World& world, Emitter& emitter) {

    for (const auto [rid, region] : world.all<ColliderRegion>()) {

        auto quad_area = region->in_rect(emitter.get_particle_bounds());

        /*
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
        */

        for (const auto& quad : quad_area) {

            if (!quad.hasAnySurface() || quad.hasOneWay)
                continue;

            auto bounds = quad.get_bounds();

            if (!bounds)
                continue;

            *bounds = math::shift(*bounds, region->getPosition());

            /*
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
            */

            /*
            for (auto &p: emitter.particles) {
                collide_particle(*region, quad, *bounds, p);
            }
            */

#if __cpp_lib_parallel_algorithm
            std::for_each(std::execution::par, emitter.particles.begin(), emitter.particles.end(),
                  [&](Particle& p){
                      collide_particle(*region, quad, *bounds, p);
                  }
            );
#else
            for (auto &p: emitter.particles) {
                collide_particle(*region, quad, *bounds, p);
            }
#endif


        }
    }
}

}