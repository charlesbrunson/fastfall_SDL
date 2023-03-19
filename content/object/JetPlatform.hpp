#pragma once

#include "fastfall/game/object/Object.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderSimple.hpp"
#include "fastfall/render/drawable/AnimatedSprite.hpp"
#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/game/path/PathMover.hpp"

class JetPlatform : public ff::Object {
public:
    static const ff::ObjectType Type;
    JetPlatform(ff::ActorInit init, ff::ObjectLevelData& data);
};
