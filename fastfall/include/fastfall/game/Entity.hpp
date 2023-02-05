#pragma once

#include "fastfall/game/ComponentID.hpp"
#include "fastfall/game/Actor.hpp"
#include "fastfall/util/copyable_uniq_ptr.hpp"

#include <set>

namespace ff {

struct EntityImGui {
    std::string name;
    std::optional<ComponentID> cmp_selected = std::nullopt;
    bool imgui_show = false;
};

struct Entity {
    std::optional<ID<Actor>> actor = {};
    std::set<ComponentID>    components;
    mutable EntityImGui      imgui;
};

}