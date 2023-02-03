#pragma once

#include "fastfall/game/object/Object.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderSimple.hpp"
#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/game/path/PathMover.hpp"

class JetPlatform : public ff::Object {
public:
    static inline const ff::ObjectType Type = {
        .name       = { "JetPlatform" },
        .anim       = std::nullopt,
        .tile_size  = {0, 1},
        .group_tags = {	"platform" },
        .properties = {
            { "path",  ff::ObjLevelID{} }
        }
    };

    JetPlatform(ff::ActorInit init, ff::ObjectLevelData& data);
    void update(ff::World& w, secs deltaTime) override;
};
