#pragma once

#include "fastfall/render/util/RenderState.hpp"
#include "fastfall/util/math.hpp"

namespace ff {

enum class scene_type {
    Object,
    Level
};

// 0 is the object layer,
// >0 is towards top (foreground)
// <0 is towards bottom (background)
using scene_layer = int;

// determines draw priority within the layer
// TODO more granularity?
enum class scene_priority {
    Lowest,
    Low,
    Normal,
    High,
    Highest
};

struct SceneConfig {
public:
    scene_layer		layer_id    = 0;
    scene_type		type        = scene_type::Object;
    scene_priority	priority    = scene_priority::Normal;
    bool            visible     = true;
    bool            resort_flag = false;

    RenderState     rstate;
    Vec2f           curr_pos;
    Vec2f           prev_pos;
    bool            auto_update_prev_pos = true;
};

}