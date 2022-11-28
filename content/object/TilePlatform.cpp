#include "TilePlatform.hpp"

using namespace ff;

const ObjectType TilePlatform::Type{
    .type = { "TilePlatform" },
    .allow_as_level_data = true,
    .anim = std::nullopt,
    .tile_size = { 0u, 0u },
    .group_tags = {	"platform" },
    .properties = {
        { "layer",	 ObjectPropertyType::Int },
    }
};

TilePlatform::TilePlatform(World& world, ID<GameObject> id)
    : GameObject(world, id)
{
};

void TilePlatform::update(World& w, secs deltaTime) {

};