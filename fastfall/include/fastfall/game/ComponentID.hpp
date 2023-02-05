#pragma once

#include "fastfall/util/id.hpp"

#include <variant>

namespace ff {

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

constexpr inline static std::string_view ComponentID_Str[] = {
    "Collidable",
    "ColliderRegion",
    "Trigger",
    "CameraTarget",
    "Drawable",
    "Emitter",
    "AttachPoint",
    "PathMover",
    "TileLayer",
};

}