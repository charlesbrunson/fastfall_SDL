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
		{"speed", ObjectPropertyType::Float},
		{"path",  ObjLevelID{}}
	}
};

BasicPlatform::BasicPlatform(World& w, ID<GameObject> id, ff::ObjectLevelData& data)
	: ff::GameObject(w, id, data)
    , shape_id{ w.create_drawable<ShapeRectangle>(id, Rectf{ Vec2f{}, Vec2f{ data.size } }, platformColor) }
    , collider_id{ w.create_collider<ColliderSimple>(id, Rectf{ Vec2f{}, Vec2f{ data.size } })}
{
    w.scene().set_config(shape_id, { 1, ff::scene_type::Object });

	ObjLevelID path_id = data.getPropAsID("path");

    auto path = data.get_sibling(path_id);

    mover_id = w.create_pathmover(id, Path{path});

    auto& mover = w.at(mover_id);
    mover.speed = data.getPropAsFloat("speed");

    auto& collider = w.at(collider_id);
    w.attach().create(w, mover.get_attach_id(), collider_id);
    w.attach().create(w, mover.get_attach_id(), shape_id);
}

void BasicPlatform::update(World& w, secs deltaTime)
{
}
