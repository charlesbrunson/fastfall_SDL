#pragma once

#include <assert.h>
#include <optional>
#include <variant>
#include <vector>

#include "fastfall/game/level/TileLayer.hpp"
#include "fastfall/game/level/ObjectLayer.hpp"

namespace ff {

struct LevelTileLayer {
    using position_t = int;

    position_t position;
    TileLayer tilelayer;
};

class LevelLayerContainer {
public:
    using position_t = LevelTileLayer::position_t;
    constexpr static position_t OBJECT_LAYER_POS = 0;

    // all return new position of tile layer
    position_t insert(position_t pos, const TileLayer& layer);
    position_t insert(position_t pos, TileLayer&& layer);

    position_t push_fg_front(const TileLayer& layer);
    position_t push_fg_front(TileLayer&& layer);

    position_t push_fg_back(const TileLayer& layer);
    position_t push_fg_back(TileLayer&& layer);

    position_t push_bg_front(const TileLayer& layer);
    position_t push_bg_front(TileLayer&& layer);

    position_t push_bg_back(const TileLayer& layer);
    position_t push_bg_back(TileLayer&& layer);

    position_t swap_next(position_t pos);
    position_t swap_prev(position_t pos);

    std::vector<int> get_id_order();
    bool reorder(std::vector<int> id_order);

    bool erase(position_t pos);
    void clear_tile_layers();
    void clear() { clear_tile_layers(); obj_layer.clear(); }

    LevelTileLayer* tile_layer_at(position_t pos);

    std::vector<LevelTileLayer>& get_tile_layers() { return tile_layers; }
    ObjectLayer& get_obj_layer() { return obj_layer; };
    int get_fg_count() { return fg_count; };
    int get_bg_count() { return bg_count; };

    std::string to_string();

private:
    int bg_count = 0;
    int fg_count = 0;
    std::vector<LevelTileLayer> tile_layers;
    ObjectLayer obj_layer;
};

}