#include "TilePlatform.hpp"

#include "fastfall/game/World.hpp"

using namespace ff;

const ActorType TilePlatform::actor_type = ActorType::create<TilePlatform>({
    .name       = "TilePlatform",
    .anim       = {},
    .tile_size  = { 0u, 0u },
    .group_tags = {	"platform" },
    .properties = {
        { "layer", ObjectProperty::Type::Int },
        { "path",  ObjLevelID{ ObjLevelID::NO_ID } }
    },
});

TilePlatform::TilePlatform(ActorInit init, const LevelObjectData& data)
    : TilePlatform(init, Rectu{ data.area } / TILESIZE, data.get_prop<int>("layer"), data.get_prop<ObjLevelID>("path"))
{
}

TilePlatform::TilePlatform(ActorInit init, Rectu area, int level_layer, ObjLevelID path_objid)
    : TileLayer(init.type_or(&actor_type), 0, area.getSize())
{
    World& w = init.world;

    // create tile layer for platform
    set_layer(w, 0);
    set_autotile_substitute("empty"_ts);
    set_collision(w, true);

    // pilfer tiles from level tile layer
    auto* active_level = w.system<LevelSystem>().get_active(w);
    auto* layer_proxy = active_level->get_tile_layer(level_layer);
    if (auto* layer = (layer_proxy ? w.get(layer_proxy->actor_id) : nullptr))
    {
        steal_tiles(w, *layer, area);
    }

    // attach tile layer to something
    ID<AttachPoint> attach_id;
    if (path_objid) {
        Path p = active_level->get_obj_layer().getObjectDataByID(path_objid);
        attach_id = w.create<PathMover>(entity_id, p)->get_attach_id();
    }
    else {
        attach_id = w.create<AttachPoint>(entity_id, id_placeholder, Vec2f{ area.topleft() } * TILESIZE_F);
    }

    auto& attachsys = w.system<AttachSystem>();
    attachsys.attach_component(w, attach_id, get_attach_id());
    attachsys.update_attachments(w, attach_id);
};
