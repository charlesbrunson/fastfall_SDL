#pragma once

#include "fastfall/render/drawable/Drawable.hpp"
#include "fastfall/util/id.hpp"

namespace ff {

class World;

void imgui_component(World& w, ID<Drawable> id);

}