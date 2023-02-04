#include "TilePlatform.hpp"

#include "fastfall/game/World.hpp"

using namespace ff;

const ObjectType TilePlatform::Type {
    .name       = { "TilePlatform" },
    .anim       = std::nullopt,
    .tile_size  = { 0u, 0u },
    .group_tags = {	"platform" },
    .properties = {
        { "layer", ff::ObjectPropertyType::Int },
        { "path",  ff::ObjLevelID{ ff::ObjLevelID::NO_ID } }
    }
};

TilePlatform::TilePlatform(ActorInit init, ObjectLevelData& data)
    : Object(init, Type, &data)
{
    World& w = init.world;

    Recti tile_area{ data.area };
    tile_area.left   /= TILESIZE;
    tile_area.top    /= TILESIZE;
    tile_area.width  /= TILESIZE;
    tile_area.height /= TILESIZE;

    // create tile layer for platform
    auto tl = w.create<TileLayer>(entity_id, w, id_placeholder, 0, (Vec2u)tile_area.getSize());
    tl->set_layer(w, 0);
    tl->set_autotile_substitute("empty"_ts);
    tl->set_collision(w, true);

    // pilfer tiles from level tile layer
    auto layer_id = data.getPropAsInt("layer");
    auto* active_level = w.system<LevelSystem>().get_active(w);
    auto* layer_proxy = active_level->get_tile_layer(layer_id);
    if (auto* layer = (layer_proxy ? w.get(layer_proxy->cmp_id) : nullptr))
    {
        tl->steal_tiles(w, *layer, tile_area);
    }

    // attach tile layer to something
    ObjLevelID path_id = data.getPropAsID("path");
    ID<AttachPoint> attach_id = path_id
        ? w.create<PathMover>(entity_id, Path{data.get_sibling(path_id)})->get_attach_id()
        : w.create<AttachPoint>(entity_id, id_placeholder, data.area.topleft());

    w.system<AttachSystem>().create(w, attach_id, tl->get_attach_id());
};
