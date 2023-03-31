#pragma once

#include "fastfall/game/actor/Actor.hpp"
#include "fastfall/game/level/TileLayer.hpp"

class TilePlatform : public ff::TileLayer {
public:
    static const ff::ActorType actor_type;
    TilePlatform(ff::ActorInit init, ff::LevelObjectData& data);
    TilePlatform(ff::ActorInit init, ff::Rectu area, int level_layer, ff::ObjLevelID path = {});
};



