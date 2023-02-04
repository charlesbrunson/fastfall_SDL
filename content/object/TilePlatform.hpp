#pragma once

#include "fastfall/game/object/Object.hpp"
#include "fastfall/game/level/TileLayer.hpp"

class TilePlatform : public ff::Object {
public:
    static inline const ff::ObjectType Type = {
        .name       = { "TilePlatform" },
        .anim       = std::nullopt,
        .tile_size  = { 0u, 0u },
        .group_tags = {	"platform" },
        .properties = {
            { "layer", ff::ObjectPropertyType::Int },
            { "path",  ff::ObjLevelID{ ff::ObjLevelID::NO_ID } }
        }
    };

    TilePlatform(ff::ActorInit init, ff::ObjectLevelData& data);
};



