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
    , shape_id{ w.create_drawable<ShapeRectangle>(id, Rectf{}, platformColor) }
    , collider_id{ w.create_collider<ColliderSimple>(id, Rectf{ Vec2f{}, Vec2f{ data.size } })}
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

	ff::Rectf colliderRect{ Vec2f{}, Vec2f{ data.size } };

    auto& collider = w.at(collider_id);
	if (has_path) {
		collider.teleport(waypoints_origin + ff::Vec2f(waypoints->at(waypoint_ndx)));
	}
	else {
		auto pos = ff::Vec2f{ data.position };
		auto off = ff::Vec2f{ colliderRect.width / 2.f, colliderRect.height };
		collider.teleport(pos - off);
	}

}

/*
void BasicPlatform::clean(ff::World& w)  {
    w.erase(shape_id);
    w.erase(collider_id);
}
*/

void BasicPlatform::update(World& w, secs deltaTime) {

    auto& collider = w.at(collider_id);
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
			collider.setPosition(ff::Vec2f(waypoints_origin) + next_pos);

			ff::Vec2f nVel = (collider.getPosition() - collider.getPrevPosition()) / deltaTime;

			collider.delta_velocity = nVel - collider.velocity;
			collider.velocity = nVel;

		}
		else if (!has_path) {

			collider.setPosition(collider.getPosition());
			collider.delta_velocity = Vec2f{};
			collider.velocity = Vec2f{};
		}
	}
}

void BasicPlatform::predraw(World& w, float interp, bool updated)
{
    auto& shape = w.at(shape_id);
    auto& collider = w.at(collider_id);
    shape.setPosition(math::lerp(collider.getPrevPosition(), collider.getPosition() + collider_offset, interp));
    shape.setSize(ff::Vec2f{ collider.getBoundingBox().getSize() });
}
