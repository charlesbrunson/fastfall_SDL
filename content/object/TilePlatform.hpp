#pragma once

#include "fastfall/game/object/Object.hpp"
#include "fastfall/game/level/TileLayer.hpp"

class TilePlatform : public ff::Object {
public:
    TilePlatform(ff::ObjectInit init, ff::ObjectLevelData& data);
};



