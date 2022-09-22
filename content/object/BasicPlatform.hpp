

#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderSimple.hpp"
#include "fastfall/game/World.hpp"

#include "fastfall/render/ShapeRectangle.hpp"

class BasicPlatform : public ff::GameObject {
public:
	static const ff::ObjectType Type;
	const ff::ObjectType& type() const override { return Type; };

	BasicPlatform(ff::World& w, ff::ObjectLevelData& data);

	void update(ff::World& w, secs deltaTime) override;

	void predraw(ff::World& w, float interp, bool updated) override;

    void clean(ff::World& w) override;

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

    ff::ID<ff::SceneObject> scene_id;
    ff::ID<ff::ColliderSimple> collider;

    ff::ShapeRectangle& shape(ff::World& w) const;
};
