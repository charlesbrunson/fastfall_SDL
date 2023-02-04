

#include "fastfall/game/object/Object.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderSimple.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/game/path/PathMover.hpp"

#include "fastfall/render/ShapeRectangle.hpp"

class BasicPlatform : public ff::Object {
public:
    static const ff::ObjectType Type;
	BasicPlatform(ff::ActorInit init, ff::ObjectLevelData& data);

protected:
    ff::ID<ff::ShapeRectangle>  shape_id;
    ff::ID<ff::ColliderSimple>  collider_id;
    ff::ID<ff::PathMover>       mover_id;
};
