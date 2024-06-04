#pragma once

#include "fastfall/game/ComponentID.hpp"

namespace ff {
    class World;

    void imgui_component_ref(const World &w, const ComponentID& cmp);
    void imgui_component(World&, ComponentID id);
}