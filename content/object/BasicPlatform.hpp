

#include "fastfall/game/GameContext.hpp"
#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderSimple.hpp"
#include "fastfall/game/CollisionManager.hpp"
#include "fastfall/game/SceneManager.hpp"

#include "fastfall/render/ShapeRectangle.hpp"

#include "fastfall/game/InstanceInterface.hpp"
#include "fastfall/game/object/ObjectComponents.hpp"

class BasicPlatform : public ff::GameObject {
public:
	BasicPlatform(ff::GameContext instance, const ff::ObjectData& ref, const ff::ObjectType& type);

	std::unique_ptr<GameObject> clone() const override;

	void update(secs deltaTime) override;

	void predraw(secs deltaTime) override;

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

	void draw(ff::RenderTarget& target, ff::RenderState states = ff::RenderState()) const override;
};
