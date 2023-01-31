#include "TilePlatform.hpp"

#include "fastfall/game/World.hpp"

using namespace ff;

const ObjectType TilePlatform::Type{
    .type = { "TilePlatform" },
    .allow_as_level_data = true,
    .anim = std::nullopt,
    .tile_size = { 0u, 0u },
    .group_tags = {	"platform" },
    .properties = {
        { "layer",	 ObjectPropertyType::Int },
        { "path",  ObjLevelID{ ObjLevelID::NO_ID } }
    }
};

TilePlatform::TilePlatform(World& w, ID<GameObject> id, ObjectLevelData& data)
    : GameObject(w, id)
{
    ObjLevelID path_id = data.getPropAsID("path");

    Rectu area{
        Vec2u{ data.getTopLeftPos() / TILESIZE },
        data.size / TILESIZE
    };

    ID<AttachPoint> attach_id;
    if (path_id) {
        auto mover_id = w.create<PathMover>(entityID(), Path{data.get_sibling(path_id)});
        attach_id = w.at(mover_id).get_attach_id();
    } else {
        attach_id = w.create<AttachPoint>(entityID(), id_placeholder, data.getTopLeftPos());
    }

    tl_id = w.create<TileLayer>(entityID(), w, id_placeholder, 0, area.getSize());
    auto& tl = w.at(tl_id);
    tl.set_layer(w, 0);
    tl.set_autotile_substitute("empty"_ts);
    tl.set_collision(w, true);

    // pilfer tiles from level layer
    unsigned layer_id = data.getPropAsInt("layer");
    auto level = w.system<LevelSystem>().get_active(w);
    auto* layer_proxy = level->get_layers().get_tile_layer_by_id(layer_id);
    if (auto* layer = (layer_proxy ? w.get(layer_proxy->cmp_id) : nullptr))
    {
        Vec2u topleft{ area.left, area.top };
        for (auto x{area.left}; x < area.left + area.width; x++)
        {
            for (auto y{area.top}; y < area.top + area.height; y++)
            {
                Vec2u p{x, y};
                if (auto tid = layer->getTileBaseID(p))
                {
                    tl.setTile(w, p - topleft, *tid, *layer->getTileTileset(p), true);
                    layer->removeTile(w, p);
                }
            }
        }
    }

    w.system<AttachSystem>().create(w, attach_id, tl.get_attach_id(), {});
};

void TilePlatform::update(World& w, secs deltaTime) {

};
