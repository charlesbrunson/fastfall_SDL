

#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderSimple.hpp"
#include "fastfall/game/World.hpp"

#include "fastfall/render/ShapeRectangle.hpp"

class BasicPlatform : public ff::GameObject {
public:
	static const ff::ObjectType Type;
	const ff::ObjectType& type() const override { return Type; };

	BasicPlatform(ff::GameContext context, ff::ObjectLevelData& data);

	void update(secs deltaTime) override;

	void predraw(float interp, bool updated) override;

protected:
	bool has_path = false;

	float max_vel = 0.f;
	float accel = 0.f;
	ff::Vec2f waypoints_origin;
	const std::vector<ff::Vec2i>* waypoints = nullptr;
	float totalDistance = 0.f;

	size_t waypoint_ndx = 0u;

	ff::Vec2f collider_offset;

	ff::Vec2f from;
	ff::Vec2f to;
	float progress = 0.f;
	bool reverser = false;

	ff::Collider_ptr<ff::ColliderSimple> collider;
	ff::Scene_ptr<ff::ShapeRectangle> shape;
	//ff::ShapeRectangle shape;
	
};
