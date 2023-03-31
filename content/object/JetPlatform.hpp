#pragma once

#include "fastfall/game/actor/Actor.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderSimple.hpp"
#include "fastfall/render/drawable/AnimatedSprite.hpp"
#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/game/path/PathMover.hpp"

class JetPlatform : public ff::Actor {
public:
    static const ff::ActorType actor_type;
    JetPlatform(ff::ActorInit init, ff::Vec2f pos, int width, ff::ObjLevelID path_objid = {});
    JetPlatform(ff::ActorInit init, ff::LevelObjectData& data);
};
