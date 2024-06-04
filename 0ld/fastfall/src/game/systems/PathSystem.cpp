#include "fastfall/game/systems/PathSystem.hpp"

#include "fastfall/game/World.hpp"

#include "fastfall/render/DebugDraw.hpp"

namespace ff {

void PathSystem::update(World& world, secs deltaTime) {
    for (auto [id, pm] : world.all<PathMover>()) {
        pm.update(world.at(pm.get_attach_id()), deltaTime);

        if (debug::enabled(debug::Path) && !debug::repeat((void*)&pm.get_path(), pm.get_path().origin))
        {
            auto path_arr = debug::draw(
                    (const void*)&pm.get_path(), Primitive::LINE_STRIP, pm.get_path().waypoints.size(), pm.get_path().origin);

            size_t n = 0;
            for (auto p : pm.get_path().waypoints)
            {
                path_arr[n].pos = p;
                path_arr[n].color = Color::White;
                ++n;
            }


            auto path_points = debug::draw(
                    (const void*)&pm.get_path(), Primitive::LINES, pm.get_path().waypoints.size() * 8, pm.get_path().origin);

            n = 0;
            for (auto p : pm.get_path().waypoints)
            {
                path_points[n].pos = p + Vec2f{-1, -1};
                path_points[n].color = Color::White;
                ++n;
                path_points[n].pos = p + Vec2f{1, -1};
                path_points[n].color = Color::White;
                ++n;
                path_points[n].pos = p + Vec2f{1, -1};
                path_points[n].color = Color::White;
                ++n;
                path_points[n].pos = p + Vec2f{1, 1};
                path_points[n].color = Color::White;
                ++n;
                path_points[n].pos = p + Vec2f{1, 1};
                path_points[n].color = Color::White;
                ++n;
                path_points[n].pos = p + Vec2f{-1, 1};
                path_points[n].color = Color::White;
                ++n;
                path_points[n].pos = p + Vec2f{-1, 1};
                path_points[n].color = Color::White;
                ++n;
                path_points[n].pos = p + Vec2f{-1, -1};
                path_points[n].color = Color::White;
                ++n;
            }
        }

        if (debug::enabled(debug::Path) && !debug::repeat((void*)&pm, pm.get_pos()))
        {
            auto path_pos = debug::draw(
                    (const void*)&pm, Primitive::LINE_LOOP, 4, pm.get_pos());

            constexpr Rectf area{
                    Vec2f{-3, -3},
                    Vec2f{6, 6}
            };

            for (auto & path_po : path_pos) {
                path_po.color = Color::White;
            }
            path_pos[0].pos = math::rect_topleft(area);
            path_pos[1].pos = math::rect_topright(area);
            path_pos[2].pos = math::rect_botright(area);
            path_pos[3].pos = math::rect_botleft(area);
        }
    }
}

void PathSystem::notify_created(World& world, ID<PathMover> id) {
}

void PathSystem::notify_erased(World& world, ID<PathMover> id) {
}

}
