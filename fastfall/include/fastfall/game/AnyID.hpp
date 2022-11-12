#pragma once

#include "fastfall/util/id.hpp"

#include <variant>

namespace ff {

class Collidable;
class ColliderRegion;
class Trigger;
class Drawable;
class Emitter;
class CameraTarget;
class AttachPoint;
class GameObject;
class Level;

using ComponentID = std::variant<
    ID<Collidable>,
    ID<ColliderRegion>,
    ID<Trigger>,
    ID<CameraTarget>,
    ID<Drawable>,
    ID<Emitter>,
    ID<AttachPoint>
>;

using EntityID = std::variant<
    ID<GameObject>,
    ID<Level>
>;

}