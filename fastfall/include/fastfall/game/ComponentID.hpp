#pragma once

#include "fastfall/util/id.hpp"

#include <variant>
#include <string_view>

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

std::string cmpid_str(const ComponentID& cmp);

class World;
void imgui_component_ref(const World& w, const ComponentID& cmp);

}