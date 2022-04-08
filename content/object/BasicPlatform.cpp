#include "BasicPlatform.hpp"

#include "fastfall/game/InstanceInterface.hpp"

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

BasicPlatform::BasicPlatform(ff::GameContext context, ff::ObjectLevelData& data)
	: ff::GameObject(context, data)
	, shape{ context, ff::ShapeRectangle{ ff::Rectf{}, platformColor}, ff::SceneType::Object, 1 }
	, collider(context, ff::Rectf{ Vec2f{}, Vec2f{ data.size } })
{

	ObjLevelID path_id = data.getPropAsID("path");
	max_vel = data.getPropAsFloat("max_velocity");
	accel = data.getPropAsFloat("acceleration");

	if (auto path_ptr = instance::lvl_get_active(m_context)->get_layers().get_obj_layer().getObjectDataByID(path_id)) {
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

	if (has_path) {
		collider->teleport(waypoints_origin + ff::Vec2f(waypoints->at(waypoint_ndx)));
	}
	else {
		auto pos = ff::Vec2f{ data.position };
		auto off = ff::Vec2f{ colliderRect.width / 2.f, colliderRect.height };
		collider->teleport(pos - off);
	}

	m_has_collider = true;

}


std::unique_ptr<ff::GameObject> BasicPlatform::clone() const {
	std::unique_ptr<BasicPlatform> object = std::make_unique<BasicPlatform>(m_context, *m_data);

	//TODO copy current state data

	return object;
}

void BasicPlatform::update(secs deltaTime) {

	if (has_path && deltaTime > 0) {


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
		collider->setPosition(ff::Vec2f(waypoints_origin) + next_pos);

		ff::Vec2f nVel = (collider->getPosition() - collider->getPrevPosition()) / deltaTime;

		collider->delta_velocity = nVel - collider->velocity;
		collider->velocity = nVel;

	}
	collider->update(deltaTime);
}

void BasicPlatform::predraw(float interp, bool updated) {
	shape->setPosition(collider->getPosition() + collider_offset);
	shape->setSize(ff::Vec2f{ collider->getBoundingBox().getSize() });
}
