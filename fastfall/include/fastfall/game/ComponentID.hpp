#pragma once

#include "fastfall/util/id.hpp"

namespace ff {

class Collidable;
class ColliderRegion;
class Trigger;
class SceneObject;
class CameraTarget;

enum class ComponentType {
    Collidable,
    Collider,
    Trigger,
    SceneObject,
    CameraTarget
};

struct ComponentID {
    ComponentID(ID<Collidable> id);
    ComponentID(ID<ColliderRegion> id);
    ComponentID(ID<Trigger> id);
    ComponentID(ID<SceneObject> id);
    ComponentID(ID<CameraTarget> id);

    ComponentType   type;
    slot_key        id_key;
};

}