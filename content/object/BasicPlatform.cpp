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
		{"acceleration", ObjectPropertyType::Float},
		{"max_velocity", ObjectPropertyType::Float},
		{"path",		 ObjLevelID{}}
	}
};

BasicPlatform::BasicPlatform(World& w, ID<GameObject> id, ff::ObjectLevelData& data)
	: ff::GameObject(w, id, data)
    , shape_id{ w.create_drawable<ShapeRectangle>(id, Rectf{ Vec2f{}, Vec2f{ data.size } }, platformColor) }
    , collider_id{ w.create_collider<ColliderSimple>(id, Rectf{ Vec2f{}, Vec2f{ data.size } })}
    , attach_id(w.create_attachpoint(id))
{
    //w.at(shape_id).drawable = make_copyable_unique<Drawable, ShapeRectangle>( Rectf{}, platformColor );
    w.scene().set_config(shape_id, { 1, ff::scene_type::Object });

	ObjLevelID path_id = data.getPropAsID("path");
	max_vel = data.getPropAsFloat("max_velocity");
	accel = data.getPropAsFloat("acceleration");

	if (auto path_ptr = data.get_sibling(path_id)) {
		waypoints_origin = ff::Vec2f{ path_ptr->position };
		waypoints = &path_ptr->points;
	}

	has_path = (waypoints);

	if (has_path) {
		if (waypoints->size() > 1) {
			totalDistance = 0.f;
			auto from = waypoints->cbegin();
			for (auto to = from + 1; to != waypoints->cend(); to++) {
				totalDistance += (ff::Vec2f{ *to } - ff::Vec2f{ *from }).magnitude();
				from = to;
			}
		}

		waypoint_ndx = 0u;
		progress = 0.f;
	}

    auto& collider = w.at(collider_id);
    auto& attach = w.at(attach_id);
    Vec2f pos;
	if (has_path) {
        pos = waypoints_origin + ff::Vec2f(waypoints->at(waypoint_ndx));
	}
	else {
        pos = data.getTopLeftPos();
	}
    attach.teleport(pos);
    w.attach().create(attach_id, collider_id, {}, {});
    w.attach().create(attach_id, shape_id, {}, {});
    //w.attach().notify(w, attach_id);
    attach.notify();
}

void BasicPlatform::update(World& w, secs deltaTime)
{
    auto& attach = w.at(attach_id);
	if (deltaTime > 0)
	{
		if (has_path)
		{
			ff::Vec2f next_pos;
			if (waypoints->size() > 1) {
				progress += deltaTime;
				if (progress > (totalDistance / max_vel)) {
					progress -= (totalDistance / max_vel);
					reverser = !reverser;
				}
				if (!reverser) {
					from = ff::Vec2f{ waypoints->at(0u) };
					to = ff::Vec2f{ waypoints->at(1u) };
				}
				else {
					from = ff::Vec2f{ waypoints->at(1u) };
					to = ff::Vec2f{ waypoints->at(0u) };
				}

				next_pos = from + ((to - from) * (progress * max_vel / totalDistance));

			}
			else {
				next_pos = ff::Vec2f{ waypoints->at(waypoint_ndx) };
			}

            attach.set_pos(Vec2f(waypoints_origin) + next_pos);
            attach.set_vel((attach.curr_pos() - attach.prev_pos()) / deltaTime);
		}
		else if (!has_path) {
            attach.set_pos(attach.curr_pos());
            attach.set_vel({});
		}
	}

    attach.notify();
    //w.attach().notify(w, attach_id);
}
