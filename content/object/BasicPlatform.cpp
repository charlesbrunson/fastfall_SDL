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

BasicPlatform::BasicPlatform(World& w, ID<GameObject> id, ff::ObjectLevelData& data)
	: ff::GameObject(w, id, data)
    , shape_id{ w.create<ShapeRectangle>(entityID(), Rectf{ Vec2f{}, Vec2f{ data.size } }, platformColor) }
    , collider_id{ w.create<ColliderSimple>(entityID(), Rectf{ Vec2f{}, Vec2f{ data.size } })}
{
    w.system<SceneSystem>().set_config(shape_id, { 1, ff::scene_type::Object });

	ObjLevelID path_id = data.getPropAsID("path");

    ID<AttachPoint> attach_id;
    if (path_id) {
        mover_id = w.create<PathMover>(entityID(), Path{data.get_sibling(path_id)});
        attach_id = w.at(mover_id).get_attach_id();
    } else {
        attach_id = w.create<AttachPoint>(entityID(), id_placeholder, data.getTopLeftPos());
    }

    w.system<AttachSystem>().create(w, attach_id, collider_id);
    w.system<AttachSystem>().create(w, attach_id, shape_id);
}

void BasicPlatform::update(World& w, secs deltaTime)
{
}
