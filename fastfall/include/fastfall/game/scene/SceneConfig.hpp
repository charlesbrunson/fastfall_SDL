#pragma once

#include "fastfall/util/id.hpp"
#include "fastfall/util/copyable_uniq_ptr.hpp"
#include "fastfall/render/Texture.hpp"

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
    scene_layer		layer_id    = 0;
    // TODO may be redundant now we have entity lookup
    scene_type		type        = scene_type::Object;
    scene_priority	priority    = scene_priority::Normal;
    std::optional<TextureRef> texture;
    bool            render_enable = true;
    bool            resort_flag   = false;
};

}