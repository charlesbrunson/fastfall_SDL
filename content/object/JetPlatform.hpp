#pragma once

#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderSimple.hpp"
#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/game/path/PathMover.hpp"

class JetPlatform : public ff::GameObject {
public:
    static const ff::ObjectType Type;
    const ff::ObjectType& type() const override { return Type; };

    JetPlatform(ff::ActorInit init, ff::ObjectLevelData& data);

    void update(ff::World& w, secs deltaTime) override;

};
