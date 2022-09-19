#pragma once

#include "fastfall/util/id.hpp"
#include "fastfall/util/copyable_uniq_ptr.hpp"

namespace ff {

class Drawable;

enum class scene_type {
    Object,
    Level
};

// 0 is the default layer,
// >0 is towards top (foreground)
// <0 is towards bottom (background)
using scene_layer = int;

// determines draw priority within the layer
enum class scene_priority {
    Lowest,
    Low,
    Normal,
    High,
    Highest
};

struct SceneObject {
    copyable_unique_ptr<Drawable> drawable;
    scene_layer		layer_id    = 0;
    scene_type		type        = scene_type::Object;
    scene_priority	priority    = scene_priority::Normal;
    bool            render_enable = true;
    bool            resort_flag   = false;
};

}