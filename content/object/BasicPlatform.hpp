

#include "fastfall/game/GameContext.hpp"
#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderSimple.hpp"
#include "fastfall/game/CollisionManager.hpp"

#include "fastfall/render/ShapeRectangle.hpp"

using namespace ff;

class BasicPlatform : public GameObject {
public:
	BasicPlatform(GameContext instance, const ObjectRef& ref) :
		GameObject(instance, ref),
		shape{
			Rectf{},
			Color{0x285cc4FF} 
		}
	{
		for (auto& [propName, propValue] : ref.properties) {
			if (propName == "path") {
				object_id obj = std::atoi(propValue.c_str());
				if (obj != object_null) {
					auto layer = ref.getLayer();
					auto c_iter = layer->objects.find(obj);

					if (c_iter != layer->objects.end()) {
						waypoints_origin = Vec2f{ c_iter->second.position };
						waypoints = &c_iter->second.points;
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
					totalDistance += (Vec2f{ *to } - Vec2f{ *from }).magnitude();
					from = to;
				}
			}

			waypoint_ndx = 0u;
			progress = 0.f;
		}

		Rectf colliderRect;
		colliderRect.width = ref.width;
		colliderRect.height = ref.height;

		collider = context.collision().get().create_collider<ColliderSimple>(colliderRect);
		if (has_path)
			collider->teleport(waypoints_origin + Vec2f(waypoints->at(waypoint_ndx)));
		else {
			collider->teleport(Vec2f{ ref.position } - Vec2f{ (float)ref.width / 2.f, (float)ref.height });
		}

		hasCollider = true;

	}

	~BasicPlatform() {
		context.collision().get().erase_collider(collider);
	}

	std::unique_ptr<GameObject> clone() const override {
		std::unique_ptr<BasicPlatform> object = std::make_unique<BasicPlatform>(context, *getObjectRef());

		//TODO copy current state data

		return object;
	}

	void update(secs deltaTime) override {

		if (has_path) {

			
			Vec2f next_pos;
			if (waypoints->size() > 1) {
				progress += deltaTime; // * max_vel / totalDistance;
				if (progress > (totalDistance / max_vel )) {
					progress -= (totalDistance / max_vel);
					reverser = !reverser;
				}
				//LOG_INFO("{}", progress);

				if (!reverser) {
					from = Vec2f{ waypoints->at(0u) };
					to = Vec2f{ waypoints->at(1u) };
				}
				else {
					from = Vec2f{ waypoints->at(1u) };
					to = Vec2f{ waypoints->at(0u) };
				}

				next_pos = from + ((to - from) * (progress * max_vel / totalDistance));

			}
			else {
				next_pos = Vec2f{ waypoints->at(waypoint_ndx) };
			}
			collider->setPosition(Vec2f(waypoints_origin) + next_pos);

			Vec2f nVel = (collider->getPosition() - collider->getPrevPosition()) / deltaTime;

			collider->delta_velocity = nVel - collider->velocity;
			collider->velocity = nVel;
			
		}
		collider->update(deltaTime);
	}

	void predraw(secs deltaTime) override {
		shape.setPosition(collider->getPosition() + collider_offset);
		shape.setSize(Vec2f{ collider->getBoundingBox().getSize() });
	}

protected:
	bool has_path = false;

	float max_vel = 0.f;
	float accel = 0.f;
	Vec2f waypoints_origin;
	const std::vector<Vec2i>* waypoints = nullptr;
	float totalDistance = 0.f;

	size_t waypoint_ndx;

	Vec2f collider_offset;

	Vec2f from;
	Vec2f to;
	float progress;
	bool reverser = false;

	ColliderSimple* collider;

	ShapeRectangle shape;

	virtual void draw(RenderTarget& target, RenderState states = RenderState()) const override {
		target.draw(shape, states);
	}
};
