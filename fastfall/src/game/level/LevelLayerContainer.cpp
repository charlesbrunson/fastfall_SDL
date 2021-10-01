#include "fastfall/game/level/LevelLayerContainer.hpp"

#include <algorithm>
#include <sstream>

namespace ff {

std::string LevelLayerContainer::to_string() {

    std::stringstream ss;

    ss << std::endl;
    ss << "size: " << tile_layers.size()
        << ", bg_count: " << bg_count
        << ", fg_count: " << fg_count
        << std::endl;

    ss << "position: ";
    auto it = tile_layers.begin();
    while (it != tile_layers.end() && it->position < 0) {
        ss << it->position << " ";
        it++;
    }
    ss << "[0] ";
    while (it != tile_layers.end()) {
        ss << it->position << " ";
        it++;
    }
    ss << std::endl;

    ss << "id:       ";
    it = tile_layers.begin();
    while (it != tile_layers.end() && it->position < 0) {
        ss << it->tilelayer.getID() << " ";
        it++;
    }
    ss << "[0] ";
    while (it != tile_layers.end()) {
        ss << it->tilelayer.getID() << " ";
        it++;
    }
    ss << std::endl;
    ss << std::endl;
    return ss.str();
}

LevelTileLayer* LevelLayerContainer::tile_layer_at(position_t pos) {
    if (pos < -bg_count || pos > fg_count) {
        return nullptr;
    }
    else if (pos < 0) {
        return &tile_layers.at((size_t)pos + bg_count);
    }
    else if (pos > 0) {
        return &tile_layers.at((size_t)pos + bg_count - 1);
    }
    else {
        return  nullptr;
    }
}

LevelLayerContainer::position_t LevelLayerContainer::insert(
    position_t pos, TileLayer&& layer)
{
    std::cout << "adding layer: " << layer.getID() << std::endl;
    assert(pos != OBJECT_LAYER_POS);
    assert(pos >= -bg_count - 1 && pos <= fg_count + 1);

    if (pos < 0) {
        int ndx = (pos + 1 + bg_count);
        if (tile_layers.empty()) {
            tile_layers.push_back(
                LevelTileLayer{ .position = pos, .tilelayer = std::move(layer) });
        }
        else {
            tile_layers.insert(
                tile_layers.begin() + ndx,
                LevelTileLayer{ .position = pos, .tilelayer = std::move(layer) });
        }

        bg_count++;
        for (auto it = tile_layers.begin(); it != tile_layers.begin() + ndx; it++) {
            it->position--;
        }
    }
    else if (pos > 0) {
        int ndx = (pos + bg_count);
        if (tile_layers.empty() || ndx == tile_layers.size() + 1) {
            tile_layers.push_back(
                LevelTileLayer{ .position = pos, .tilelayer = std::move(layer) });
        }
        else {
            tile_layers.insert(
                tile_layers.begin() + ndx,
                LevelTileLayer{ .position = pos, .tilelayer = std::move(layer) });
        }

        fg_count++;
        for (auto it = tile_layers.begin() + ndx; it != tile_layers.end();
            it++) {
            it->position++;
        }
    }
    return pos;
}

LevelLayerContainer::position_t LevelLayerContainer::insert(
    position_t pos, const TileLayer& layer)
{
    TileLayer n_layer = layer;
    return insert(pos, std::move(n_layer));
}

LevelLayerContainer::position_t LevelLayerContainer::push_fg_front(const TileLayer& layer)
{
    return insert(fg_count + 1, layer);
}

LevelLayerContainer::position_t LevelLayerContainer::push_fg_front(TileLayer&& layer)
{
    return insert(fg_count + 1, std::move(layer));
}

LevelLayerContainer::position_t LevelLayerContainer::push_fg_back(const TileLayer& layer)
{
    return insert(1, layer);
}

LevelLayerContainer::position_t LevelLayerContainer::push_fg_back(TileLayer&& layer)
{
    return insert(1, std::move(layer));
}

LevelLayerContainer::position_t LevelLayerContainer::push_bg_front(const TileLayer& layer)
{
    return insert(-1, layer);
}

LevelLayerContainer::position_t LevelLayerContainer::push_bg_front(TileLayer&& layer)
{
    return insert(-1, std::move(layer));
}

LevelLayerContainer::position_t LevelLayerContainer::push_bg_back(const TileLayer& layer)
{
    return insert(-bg_count - 1, layer);
}

LevelLayerContainer::position_t LevelLayerContainer::push_bg_back(TileLayer&& layer)
{
    return insert(-bg_count - 1, std::move(layer));
}

LevelLayerContainer::position_t LevelLayerContainer::swap_next(position_t pos)
{
    assert(pos >= -bg_count && pos <= fg_count);

    if (pos == 0) {
        if (fg_count > 0) {
            swap_prev(1);
        }
    }
    else {
        if (pos == -1) {

            tile_layer_at(-1)->position = 1;

            bg_count--;
            fg_count++;

            for (int i = -bg_count; i <= fg_count; i++) {
                if (i == 0) continue;
                tile_layer_at(i)->position = i;
            }
            return 1;
        }
        else {
            auto layer = tile_layer_at(pos);
            auto next = tile_layer_at(pos + 1);

            if (layer && next) {
                std::swap(
                    layer->tilelayer,
                    next->tilelayer
                );
                return next->position;
            }
        }
    }
    return pos;
}

LevelLayerContainer::position_t LevelLayerContainer::swap_prev(position_t pos)
{
    if (pos == 0) {
        if (bg_count > 0) {
            swap_next(-1);
            return 0;
        }
    }
    else {
        if (pos == 1) {
            tile_layer_at(1)->position = -1;

            bg_count++;
            fg_count--;


            for (int i = -bg_count; i <= fg_count; i++) {
                if (i == 0) continue;
                tile_layer_at(i)->position = i;
            }

            return -1;
        }
        else {
            auto layer = tile_layer_at(pos);
            auto prev = tile_layer_at(pos - 1);

            if (layer && prev) {
                std::swap(
                    layer->tilelayer,
                    prev->tilelayer
                );
                return prev->position;
            }
        }
    }

    return position_t();
}

bool LevelLayerContainer::erase(position_t pos)
{
    if (pos != 0) 
    {
        auto layer = tile_layer_at(pos);

        if (layer) 
        {
           // auto layer = get<LevelLayer*>(value);

            int ndx = pos + bg_count - (pos > 0 ? 1 : 0);
            tile_layers.erase(tile_layers.begin() + ndx);

            (pos > 0 ? fg_count : bg_count)--;

            for (int i = -bg_count; i <= fg_count; i++) 
            {
                if (i == 0) continue;
                tile_layer_at(i)->position = i;
            }

            return true;
        }
    }
    else 
    {
        // TODO

        return true;
    }
    return false;
}


std::vector<int> LevelLayerContainer::get_id_order()
{
    std::vector<int> ret;
    ret.reserve(tile_layers.size() + 1);

    for (int i = -bg_count; i <= fg_count; i++) {
        if (i == 0) {
            ret.push_back(0);
        }
        else {
            ret.push_back(tile_layer_at(i)->tilelayer.getID());
        }
    }

    return ret;
}

bool LevelLayerContainer::reorder(std::vector<int> id_order)
{
    auto zero_it = std::find(id_order.begin(), id_order.end(), 0);

    if (zero_it == id_order.end())
        return false;

    std::erase_if(id_order, [this](int id) {
        return id != 0 && std::find_if(tile_layers.begin(), tile_layers.end(), [id](const LevelTileLayer& layer) {
            return layer.tilelayer.getID() == id;
            }) == tile_layers.end();
        });

    if (id_order.size() - 1 != tile_layers.size())
        return false;

    int n_bg_count = std::distance(id_order.begin(), zero_it);
    int n_fg_count = id_order.size() - n_bg_count - 1;

    // reassign positions
    for (auto& layer : tile_layers)
    {
        layer.position = std::distance(
            id_order.begin(),
            std::find(id_order.begin(), id_order.end(), layer.tilelayer.getID())
        ) - n_bg_count;
    }

    std::sort(tile_layers.begin(), tile_layers.end(),
        [](const LevelTileLayer& lhs, const LevelTileLayer& rhs) {
            return lhs.position < rhs.position;
        });

    bg_count = n_bg_count;
    fg_count = n_fg_count;
    return true;
}

void LevelLayerContainer::clear_tile_layers()
{
    tile_layers.clear();
    bg_count = 0;
    fg_count = 0;
}

}
