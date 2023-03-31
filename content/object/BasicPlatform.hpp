
#include "fastfall/game/actor/Actor.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderSimple.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/game/path/PathMover.hpp"

#include "fastfall/render/drawable/ShapeRectangle.hpp"

class BasicPlatform : public ff::Actor {
public:
    static const ff::ActorType actor_type;

    BasicPlatform(ff::ActorInit init, ff::LevelObjectData& data);
	BasicPlatform(ff::ActorInit init, ff::Rectf area, ff::ObjLevelID path_objid = {});

protected:
    ff::ID<ff::ShapeRectangle>  shape_id;
    ff::ID<ff::ColliderSimple>  collider_id;
    ff::ID<ff::PathMover>       mover_id;
};
