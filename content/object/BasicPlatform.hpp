

#include "fastfall/game/object/GameObject.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderSimple.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/game/path/PathMover.hpp"

#include "fastfall/render/ShapeRectangle.hpp"

class BasicPlatform : public ff::GameObject {
public:
	static const ff::ObjectType Type;
	const ff::ObjectType& type() const override { return Type; };

	BasicPlatform(ff::ActorInit init, ff::ObjectLevelData& data);
	void update(ff::World& w, secs deltaTime) override;

protected:
    ff::ID<ff::ShapeRectangle> shape_id;
    ff::ID<ff::ColliderSimple> collider_id;
    ff::ID<ff::PathMover> mover_id;

};
