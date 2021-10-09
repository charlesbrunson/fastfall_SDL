#include "BasicPlatform.hpp"

#include "fastfall/game/InstanceInterface.hpp"

constexpr ff::Color platformColor = ff::Color{ 0x285cc4FF };

BasicPlatform::BasicPlatform(ff::GameContext instance, const ff::ObjectData& ref, const ff::ObjectType& type)
	: ff::GameObject(	instance, ref, type)
	, shape{			instance, ff::ShapeRectangle{ ff::Rectf{}, platformColor}, ff::SceneType::Object, 1 }
	, collider(			instance, ff::Rectf{ 0.f, 0.f, (float)ref.width, (float)ref.height })
{
	for (auto& [propName, propValue] : ref.properties) {
		if (propName == "path") {
			ff::object_id path_id = std::atoi(propValue.c_str());

			if (auto lvl = ff::instance::lvl_get_active(context)) {
				if (auto ref_ptr = lvl->get_layers().get_obj_layer().getRefByID(path_id)) {
					waypoints_origin = ff::Vec2f{ ref_ptr->position };
					waypoints = &ref_ptr->points;
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

	ff::Rectf colliderRect;
	colliderRect.width = ref.width;
	colliderRect.height = ref.height;

	if (has_path)
		collider->teleport(waypoints_origin + ff::Vec2f(waypoints->at(waypoint_ndx)));
	else {
		collider->teleport(ff::Vec2f{ ref.position } - ff::Vec2f{ (float)ref.width / 2.f, (float)ref.height });
	}

	hasCollider = true;

}


std::unique_ptr<ff::GameObject> BasicPlatform::clone() const {
	std::unique_ptr<BasicPlatform> object = std::make_unique<BasicPlatform>(context, getObjectRef(), getType());

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

void BasicPlatform::draw(ff::RenderTarget& target, ff::RenderState states) const {
	target.draw(*shape, states);
}