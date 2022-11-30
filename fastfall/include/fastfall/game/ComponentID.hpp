#pragma once

#include "fastfall/util/id.hpp"

#include <variant>

namespace ff {

class GameObject;
class Level;
class Collidable;
class ColliderRegion;
class Trigger;
class CameraTarget;
class Drawable;
class Emitter;
class AttachPoint;
class PathMover;
class TileLayer;

using ComponentID = std::variant<
    ID<GameObject>,
    ID<Level>,
    ID<Collidable>,
    ID<ColliderRegion>,
    ID<Trigger>,
    ID<CameraTarget>,
    ID<Drawable>,
    ID<Emitter>,
    ID<AttachPoint>,
    ID<PathMover>,
    ID<TileLayer>
>;

}