#pragma once


#include "fastfall/game/object/Object.hpp"
#include "fastfall/game/level/TileLayer.hpp"

#include <memory>

class TilePlatform : public ff::Object {
public:
    TilePlatform(ff::ActorInit init, ff::ObjectLevelData& data);

private:
    ff::ID<ff::TileLayer> tl_id;
};

