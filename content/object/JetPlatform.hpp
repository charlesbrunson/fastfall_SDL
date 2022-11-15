#pragma once

#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderSimple.hpp"
#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/game/path/PathFollower.hpp"

class JetPlatform : public ff::GameObject {
public:
    static const ff::ObjectType Type;
    const ff::ObjectType& type() const override { return Type; };

    JetPlatform(ff::World& w, ff::ID<ff::GameObject> id, ff::ObjectLevelData& data);

    void update(ff::World& w, secs deltaTime) override;

protected:
    ff::PathFollower path_follower;
    ff::ID<ff::AttachPoint> base_attach_id;
    //ff::ID<ff::AttachPoint> attach_id;
    //ff::ID<ff::ColliderTileMap> collider_id;
    //ff::ID<ff::AnimatedSprite> sprite_id;
    //ff::ID<ff::Emitter> emitter_id;
};
