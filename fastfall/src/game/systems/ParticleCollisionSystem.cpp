
#include "fastfall/game/systems/ParticleCollisionSystem.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/render/DebugDraw.hpp"

#include <chrono>

#if __cpp_lib_parallel_algorithm
#include <execution>
#endif

namespace ff {

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
    // LOG_INFO("\t{}ms - {} particles", elapsed.count(), particle_count);
}

void ParticleCollisionSystem::collide_emitter_particles(const World& world, Emitter& emitter) {

    for (const auto [rid, region] : world.all<ColliderRegion>()) {

        auto quad_area = region->in_rect(emitter.get_particle_bounds());


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
            std::for_each(std::execution::par, emitter.particles.begin(), emitter.particles.end(),
                  [&](Particle& p){
                      collide_quad(*region, quad, p);
                  }
            );
#else
            for (auto &p: emitter.particles) {
                collide_quad(*region, quad, p);
            }
#endif


        }
    }
}

}