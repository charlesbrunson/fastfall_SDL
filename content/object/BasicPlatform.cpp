#include "BasicPlatform.hpp"

using namespace ff;

constexpr ff::Color platformColor = ff::Color{ 0x285cc4FF };

const ObjectType BasicPlatform::Type{
	.type = { "BasicPlatform" },
	.allow_as_level_data = true,
	.anim = std::nullopt,
	.tile_size = {0, 0},
	.group_tags = {	"platform" },
	.properties = {
		{ "path",  ObjLevelID{ ObjLevelID::NO_ID } }
	}
};

BasicPlatform::BasicPlatform(ActorInit init, ff::ObjectLevelData& data)
	: ff::Object(init, data)
    , shape_id{ init.world.create<ShapeRectangle>(init.entity_id, Rectf{ Vec2f{}, Vec2f{ data.size } }, platformColor) }
    , collider_id{ init.world.create<ColliderSimple>(init.entity_id, Rectf{ Vec2f{}, Vec2f{ data.size } })}
{
    World& w = init.world;
    w.system<SceneSystem>().set_config(shape_id, { 1, ff::scene_type::Object });

	ObjLevelID path_id = data.getPropAsID("path");

    ID<AttachPoint> attach_id;
    if (path_id) {
        mover_id = w.create<PathMover>(init.entity_id, Path{data.get_sibling(path_id)});
        attach_id = w.at(mover_id).get_attach_id();
    } else {
        attach_id = w.create<AttachPoint>(init.entity_id, id_placeholder, data.getTopLeftPos());
    }

    w.system<AttachSystem>().create(w, attach_id, collider_id);
    w.system<AttachSystem>().create(w, attach_id, shape_id);
}

void BasicPlatform::update(World& w, secs deltaTime)
{
}
