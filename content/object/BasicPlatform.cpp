#include "BasicPlatform.hpp"

using namespace ff;

constexpr ff::Color platformColor = ff::Color{ 0x285cc4FF };

const ActorType BasicPlatform::actor_type = ActorType::create<BasicPlatform>({
    .name       = "BasicPlatform",
    .anim       = {},
    .tile_size  = {0, 0},
    .group_tags = {	"platform" },
    .properties = {
        { "path",  ff::ObjLevelID::NO_ID }
    },
});

BasicPlatform::BasicPlatform(ActorInit init, const ff::LevelObjectData& data)
    : BasicPlatform(init, data.area, data.get_prop<ObjLevelID>("path"))
{
}

BasicPlatform:: BasicPlatform(ff::ActorInit init, ff::Rectf area, ff::ObjLevelID path_objid)
    : Actor(init.type_or(&actor_type))
    , shape_id( init.world.create<ShapeRectangle>(init.entity_id, Rectf{ {}, Vec2f{ area.getSize() } }, platformColor) )
    , collider_id( init.world.create<ColliderSimple>(init.entity_id, Rectf{ {}, Vec2f{ area.getSize() } }) )
{
    World& w = init.world;
    w.system<SceneSystem>().set_config(shape_id, { 1, ff::scene_type::Object });

    Level* active_level = w.system<LevelSystem>().get_active(w);
    ID<AttachPoint> attach_id;
    if (active_level && path_objid) {
        Path p = active_level->get_obj_layer().getObjectDataByID(path_objid);
        mover_id = w.create<PathMover>(init.entity_id, p);
        attach_id = w.at(mover_id).get_attach_id();
    } else {
        attach_id = w.create<AttachPoint>(init.entity_id, id_placeholder, area.topleft());
    }

    w.system<AttachSystem>().create(w, attach_id, collider_id);
    w.system<AttachSystem>().create(w, attach_id, shape_id);
}


