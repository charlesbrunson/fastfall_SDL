

#include "fastfall/game/object/Object.hpp"
#include "fastfall/game/phys/collider_regiontypes/ColliderSimple.hpp"
#include "fastfall/game/World.hpp"
#include "fastfall/game/path/PathMover.hpp"

#include "fastfall/render/ShapeRectangle.hpp"

class BasicPlatform : public ff::Object {
public:
    static inline const ff::ObjectType Type = {
        .name       = { "BasicPlatform" },
        .anim       = std::nullopt,
        .tile_size  = {0, 0},
        .group_tags = {	"platform" },
        .properties = {
            { "path",  ff::ObjLevelID{} }
        }
    };

	BasicPlatform(ff::ActorInit init, ff::ObjectLevelData& data);
	void update(ff::World& w, secs deltaTime) override;

protected:
    ff::ID<ff::ShapeRectangle>  shape_id;
    ff::ID<ff::ColliderSimple>  collider_id;
    ff::ID<ff::PathMover>       mover_id;

};
