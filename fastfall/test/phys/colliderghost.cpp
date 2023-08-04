

#include "fastfall/game/phys/collider_regiontypes/ColliderTileMap.hpp"
#include "fastfall/game/phys/collider_coretypes/ColliderQuad.hpp"
#include "gtest/gtest.h"

using namespace ff;

TEST(colliderghost, slope_to_oneway_right) {
    ColliderTileMap map{Vec2i{ 2, 2 }};
    map.setTile(Vec2i{0, 0}, "slope-h"_ts);
    map.setTile(Vec2i{0, 1}, "solid"_ts);
    map.setTile(Vec2i{1, 1}, "oneway"_ts);
    map.applyChanges();

    std::vector<const ColliderQuad *> nearby;
    nearby.push_back(map.get_quad(Vec2i{ 0, 0 }));
    nearby.push_back(map.get_quad(Vec2i{ 1, 0 }));
    nearby.push_back(map.get_quad(Vec2i{ 0, 1 }));
    nearby.push_back(map.get_quad(Vec2i{ 1, 1 }));

    ColliderSurface surf = *map.get_quad(Vec2i{ 0, 0 })->getSurface(Cardinal::N);
    auto collider = findColliderGhosts(nearby, surf);

    EXPECT_EQ( collider.ghostp3.x, 32 );
    EXPECT_EQ( collider.ghostp3.y, 16 );
}

TEST(colliderghost, slope_to_oneway_left) {
    ColliderTileMap map{Vec2i{ 2, 2 }};
    map.setTile(Vec2i{ 1, 0 }, "slope"_ts);
    map.setTile(Vec2i{ 1, 1 }, "solid"_ts);
    map.setTile(Vec2i{ 0, 1 }, "oneway"_ts);
    map.applyChanges();

    std::vector<const ColliderQuad *> nearby;
    nearby.push_back(map.get_quad(Vec2i{ 0, 0 }));
    nearby.push_back(map.get_quad(Vec2i{ 1, 0 }));
    nearby.push_back(map.get_quad(Vec2i{ 0, 1 }));
    nearby.push_back(map.get_quad(Vec2i{ 1, 1 }));

    ColliderSurface surf = *map.get_quad(Vec2i{ 1, 0 })->getSurface(Cardinal::N);
    auto collider = findColliderGhosts(nearby, surf);

    EXPECT_EQ( collider.ghostp0.x, 0 );
    EXPECT_EQ( collider.ghostp0.y, 16 );
}
