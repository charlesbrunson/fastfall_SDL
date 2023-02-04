#include "TilePlatform.hpp"

#include "fastfall/game/World.hpp"

using namespace ff;

TilePlatform::TilePlatform(ObjectInit init, ObjectLevelData& data)
    : Object(init)
{
    World& w = init.world;

    // create tile layer for platform
    auto tl = w.create<TileLayer>(entity_id, w, id_placeholder, 0, (Vec2u)data.area.getSize());
    tl->set_layer(w, 0);
    tl->set_autotile_substitute("empty"_ts);
    tl->set_collision(w, true);

    // pilfer tiles from level tile layer
    auto layer_id = data.getPropAsInt("layer");
    auto* active_level = w.system<LevelSystem>().get_active(w);
    auto* layer_proxy = active_level->get_tile_layer(layer_id);
    if (auto* layer = (layer_proxy ? w.get(layer_proxy->cmp_id) : nullptr))
    {
        tl->pilfer(w, *layer, data.area);
    }

    // attach tile layer to something
    ObjLevelID path_id = data.getPropAsID("path");
    ID<AttachPoint> attach_id = path_id
        ? w.create<PathMover>(entity_id, Path{data.get_sibling(path_id)})->get_attach_id()
        : w.create<AttachPoint>(entity_id, id_placeholder, data.area.topleft());

    w.system<AttachSystem>().create(w, attach_id, tl->get_attach_id(), {});
};

