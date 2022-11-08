#pragma once

#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderSimple.hpp"
#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/World.hpp"

class JetPlatform : public ff::GameObject {
public:
    static const ff::ObjectType Type;
    const ff::ObjectType& type() const override { return Type; };

    JetPlatform(ff::World& w, ff::ID<ff::GameObject> id, ff::ObjectLevelData& data);

    void update(ff::World& w, secs deltaTime) override;
    void predraw(ff::World& w, float interp, bool updated) override;
    void clean(ff::World& w) override;

protected:
    ff::ID<ff::SceneObject> scene_id;
    ff::ID<ff::ColliderTileMap> collider_id;
    ff::ID<ff::AnimatedSprite> sprite_id;
    ff::Vec2f base_position;
    secs lifetime = 0.0;

    ff::Vec2f position;
    ff::Vec2f velocity;
    ff::Vec2f push_accum;
    float push_accel;
    ff::Vec2f offset;
    unsigned tile_width = 3;
};
