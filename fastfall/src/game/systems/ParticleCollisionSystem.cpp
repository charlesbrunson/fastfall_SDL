
#include "fastfall/game/systems/ParticleCollisionSystem.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/render/DebugDraw.hpp"

namespace ff {

void collide_particle(const ColliderQuad& quad, Particle& particle) {

}

void ParticleCollisionSystem::update(World& world, secs deltaTime) {
    for (auto [id, e] : world.all<Emitter>()) {
        if (e.strategy.has_collision) {
            collide_emitter_particles(world, e);
        }
    }
}

void ParticleCollisionSystem::collide_emitter_particles(World& world, Emitter& emitter) {

    for (auto [rid, region] : world.all<ColliderRegion>()) {

        auto quad_area = region->in_rect(emitter.get_particle_bounds());

        if (debug_draw::hasTypeEnabled(debug_draw::Type::EMITTER)) {
            auto it = quad_area.begin();
            Rectf r_bounds = math::shift(Rectf{ it.get_tile_area() } * TILESIZE, region->getPosition());

            auto& p_bounds = createDebugDrawable<VertexArray, debug_draw::Type::EMITTER>(
                    (const void*)this, Primitive::LINE_LOOP, 4);

            for (int i = 0; i < p_bounds.size(); i++) {
                p_bounds[i].color = Color::White;
            }
            p_bounds[0].pos = math::rect_topleft(r_bounds);
            p_bounds[1].pos = math::rect_topright(r_bounds);
            p_bounds[2].pos = math::rect_botright(r_bounds);
            p_bounds[3].pos = math::rect_botleft(r_bounds);
        }

        for (auto& quad : quad_area) {

            if (!quad.hasAnySurface() || quad.hasOneWay)
                continue;

            auto bounds = quad.get_bounds();

            if (!bounds)
                continue;

            *bounds = math::shift(*bounds, region->getPosition());

            if (debug_draw::hasTypeEnabled(debug_draw::Type::EMITTER)) {
                auto& q_bounds = createDebugDrawable<VertexArray, debug_draw::Type::EMITTER>(
                        (const void*)this, Primitive::LINE_LOOP, 4);

                for (int i = 0; i < q_bounds.size(); i++) {
                    q_bounds[i].color = Color::Green;
                }
                q_bounds[0].pos = math::rect_topleft(*bounds);
                q_bounds[1].pos = math::rect_topright(*bounds);
                q_bounds[2].pos = math::rect_botright(*bounds);
                q_bounds[3].pos = math::rect_botleft(*bounds);
            }

            for (auto &p: emitter.particles) {
                collide_particle(quad, p);
            }
        }
    }
}

}