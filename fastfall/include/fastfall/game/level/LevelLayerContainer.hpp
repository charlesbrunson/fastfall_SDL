#pragma once

#include <assert.h>
#include <optional>
#include <variant>
#include <vector>

#include <algorithm>
#include <sstream>

#include "fastfall/game/level/TileLayer.hpp"
#include "fastfall/game/level/ObjectLayer.hpp"

namespace ff {

template<typename T>
concept HasID = requires (T x) {
    { x.getID() } -> std::convertible_to<int>;
};



template<HasID TileLayerT, HasID ObjLayerT>
class LevelLayerContainer {
public:
    using position_t = int;

    struct TileEntry {
        position_t position;
        TileLayerT tilelayer;
    };

    constexpr static position_t OBJECT_LAYER_POS = 0;

    // all return new position of tile layer
    position_t insert(position_t pos, TileLayerT&& layer)
    {
        if (pos == OBJECT_LAYER_POS || pos < -bg_count - 1 || pos > fg_count + 1)
        {
            throw std::out_of_range("index out of range");
        }
        if (pos < 0) {
            int ndx = (pos + 1 + bg_count);
            if (tile_layers.empty()) {
                tile_layers.push_back(
                    TileEntry{ .position = pos, .tilelayer = std::move(layer) });
            }
            else {
                tile_layers.insert(
                    tile_layers.begin() + ndx,
                    TileEntry{ .position = pos, .tilelayer = std::move(layer) });
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
                    TileEntry{ .position = pos, .tilelayer = std::move(layer) });
            }
            else {
                tile_layers.insert(
                    tile_layers.begin() + ndx,
                    TileEntry{ .position = pos, .tilelayer = std::move(layer) });
            }

            fg_count++;
            for (auto it = tile_layers.begin() + ndx; it != tile_layers.end();
                it++) {
                it->position++;
            }
        }
        return pos;
    }
    position_t insert(position_t pos, const TileLayerT& layer)
    {
        return insert(pos, TileLayer{ layer });
    }

    position_t push_fg_front(const TileLayerT& layer)
    {
        return insert(fg_count + 1, layer);
    }
    position_t push_fg_front(TileLayerT&& layer)
    {
        return insert(fg_count + 1, std::move(layer));
    }

    position_t push_fg_back(const TileLayerT& layer)
    {
        return insert(1, layer);
    }
    position_t push_fg_back(TileLayerT&& layer)
    {
        return insert(1, std::move(layer));
    }

    position_t push_bg_front(const TileLayerT& layer)
    {
        return insert(-1, layer);
    }
    position_t push_bg_front(TileLayerT&& layer)
    {
        return insert(-1, std::move(layer));
    }

    position_t push_bg_back(const TileLayerT& layer)
    {
        return insert(-bg_count - 1, layer);
    }
    position_t push_bg_back(TileLayerT&& layer)
    {
        return insert(-bg_count - 1, std::move(layer));
    }

    position_t swap_next(position_t pos)
    {
        if (pos < -bg_count - 1 || pos > fg_count + 1)
        {
            throw std::out_of_range("index out of range");
        }

        if (pos == 0) {
            if (fg_count > 0) {
                swap_prev(1);
            }
        }
        else {
            if (pos == -1) {

                get_tile_layer_at(-1)->position = 1;

                bg_count--;
                fg_count++;

                for (int i = -bg_count; i <= fg_count; i++) {
                    if (i == 0) continue;
                    get_tile_layer_at(i)->position = i;
                }
                return 1;
            }
            else {
                auto layer = get_tile_layer_at(pos);
                auto next = get_tile_layer_at(pos + 1);

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
    position_t swap_prev(position_t pos)
    {
        if (pos < -bg_count - 1 || pos > fg_count + 1)
        {
            throw std::out_of_range("index out of range");
        }

        if (pos == 0) {
            if (bg_count > 0) {
                swap_next(-1);
                return 0;
            }
        }
        else {
            if (pos == 1) {
                get_tile_layer_at(1)->position = -1;

                bg_count++;
                fg_count--;


                for (int i = -bg_count; i <= fg_count; i++) {
                    if (i == 0) continue;
                    get_tile_layer_at(i)->position = i;
                }

                return -1;
            }
            else {
                auto layer = get_tile_layer_at(pos);
                auto prev = get_tile_layer_at(pos - 1);

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

    std::vector<int> get_id_order()
    {
        std::vector<int> ret{ tile_layers.size() + 1, 0 };
        for (int i = -bg_count; i <= fg_count; i++) 
        {
            if (i == 0) 
                continue;

            ret.at(i) = get_tile_layer_at(i)->tilelayer.getID();
        }
        return ret;
    }

    bool reorder(std::vector<int> id_order)
    {
        auto zero_it = std::find(id_order.begin(), id_order.end(), 0);

        if (zero_it == id_order.end())
            return false;

        std::erase_if(id_order, [this](int id) {
            return id != 0 && std::find_if(tile_layers.begin(), tile_layers.end(), [id](const TileEntry& layer) {
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
            [](const TileEntry& lhs, const TileEntry& rhs) {
                return lhs.position < rhs.position;
            });

        bg_count = n_bg_count;
        fg_count = n_fg_count;
        return true;
    }

    bool erase(position_t pos)
    {
        if (pos != 0)
        {
            auto layer = get_tile_layer_at(pos);
            if (layer)
            {
                int ndx = pos + bg_count - (pos > 0 ? 1 : 0);
                tile_layers.erase(tile_layers.begin() + ndx);

                (pos > 0 ? fg_count : bg_count)--;

                for (int i = -bg_count; i <= fg_count; i++)
                {
                    if (i == 0) continue;
                    get_tile_layer_at(i)->position = i;
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

    void clear_tile_layers()
    {
        tile_layers.clear();
        bg_count = 0;
        fg_count = 0;
    }
    void clear_obj_layer()
    {
        obj_layer.clear();
    }
    void clear_all() { clear_tile_layers(); clear_obj_layer(); }

    TileEntry* get_tile_layer_at(position_t pos)
    {
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

    std::vector<TileEntry>& get_tile_layers() { return tile_layers; }
    const std::vector<TileEntry>& get_tile_layers() const { return tile_layers; }

    ObjLayerT& get_obj_layer() { return obj_layer; };
    const ObjLayerT& get_obj_layer() const { return obj_layer; };

    int get_fg_count() const  { return fg_count; };
    int get_bg_count() const  { return bg_count; };

    std::string to_string() 
    {
        std::stringstream ss;
        ss << std::endl << "size: " << tile_layers.size() << ", bg_count: " 
            << bg_count << ", fg_count: " << fg_count << std::endl;

        ss << "position: ";
        auto it = tile_layers.begin();
        for (; it != tile_layers.end() && it->position < 0; it++) {
            ss << it->position << " ";
        }
        ss << "[0] ";
        for (; it != tile_layers.end(); it++) {
            ss << it->position << " ";
        }
        ss << std::endl;

        ss << "id:       ";
        it = tile_layers.begin();
        for (; it != tile_layers.end() && it->position < 0; it++) {
            ss << it->tilelayer.getID() << " ";
        }
        ss << "[0] ";
        for (; it != tile_layers.end(); it++) {
            ss << it->tilelayer.getID() << " ";
        }
        ss << std::endl << std::endl;
        return ss.str();
    }

private:
    int bg_count = 0;
    int fg_count = 0;
    std::vector<TileEntry> tile_layers;
    ObjLayerT obj_layer;
};

}