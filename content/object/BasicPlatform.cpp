#include "BasicPlatform.hpp"

#include "fastfall/game/InstanceInterface.hpp"

using namespace ff;

constexpr ff::Color platformColor = ff::Color{ 0x285cc4FF };

BasicPlatform::BasicPlatform(ff::ObjectConfig cfg)
	: ff::GameObject(cfg)
	, shape{ cfg.context, ff::ShapeRectangle{ ff::Rectf{}, platformColor}, ff::SceneType::Object, 1 }
	, collider( cfg.context, ff::Rectf{ Vec2f{}, Vec2f{ cfg.m_level_data->size } })
{

	ObjLevelID path_id = cfg.m_level_data->getPropAsID("path");
	max_vel = cfg.m_level_data->getPropAsFloat("max_velocity");
	accel = cfg.m_level_data->getPropAsFloat("acceleration");

	if (auto path_ptr = instance::lvl_get_active(getContext())->get_layers().get_obj_layer().getObjectDataByID(path_id)) {
		waypoints_origin = ff::Vec2f{ path_ptr->position };
		waypoints = &path_ptr->points;
	}


	for (auto& [propName, propValue] : cfg.m_level_data->properties) {
		if (propName == "path") {
			ObjLevelID path_id = ObjLevelID{ (unsigned)std::atoi(propValue.c_str()) };

			if (auto lvl = ff::instance::lvl_get_active(getContext())) {
				if (auto data_ptr = lvl->get_layers().get_obj_layer().getObjectDataByID(path_id)) {
					waypoints_origin = ff::Vec2f{ data_ptr->position };
					waypoints = &data_ptr->points;
				}
			}
		}
		else if (propName == "max_velocity") {
			max_vel = std::atof(propValue.c_str());
		}
		else if (propName == "acceleration") {
			accel = std::atof(propValue.c_str());
		}
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

	ff::Rectf colliderRect{ Vec2f{}, Vec2f{ cfg.m_level_data->size } };

	if (has_path) {
		collider->teleport(waypoints_origin + ff::Vec2f(waypoints->at(waypoint_ndx)));
	}
	else {
		auto pos = ff::Vec2f{ cfg.m_level_data->position };
		auto off = ff::Vec2f{ colliderRect.width / 2.f, colliderRect.height };
		collider->teleport(pos - off);
	}

	hasCollider = true;

}


std::unique_ptr<ff::GameObject> BasicPlatform::clone() const {
	std::unique_ptr<BasicPlatform> object = std::make_unique<BasicPlatform>(getConfig());

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

void BasicPlatform::predraw(secs deltaTime) {
	shape->setPosition(collider->getPosition() + collider_offset);
	shape->setSize(ff::Vec2f{ collider->getBoundingBox().getSize() });
}
