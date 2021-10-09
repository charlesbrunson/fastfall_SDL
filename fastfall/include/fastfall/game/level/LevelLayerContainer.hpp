#pragma once

#include <assert.h>
#include <optional>
#include <variant>
#include <vector>

#include <algorithm>
#include <sstream>


namespace ff {

template<typename T>
concept HasID = requires (T x) {
    { x.getID() } -> std::same_as<unsigned>;
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

private:
    int bg_count = 0;
    int fg_count = 0;
    std::vector<TileEntry> tile_layers;
    ObjLayerT obj_layer;

public:
    position_t insert(position_t pos, TileLayerT&& layer);
    position_t insert(position_t pos, const TileLayerT& layer);

    position_t push_fg_front(const TileLayerT& layer);
    position_t push_fg_front(TileLayerT&& layer);

    position_t push_fg_back(const TileLayerT& layer);
    position_t push_fg_back(TileLayerT&& layer);

    position_t push_bg_front(const TileLayerT& layer);
    position_t push_bg_front(TileLayerT&& layer);

    position_t push_bg_back(const TileLayerT& layer);
    position_t push_bg_back(TileLayerT&& layer);

    position_t swap_next(position_t pos);
    position_t swap_prev(position_t pos);

    std::vector<int> get_id_order() const;

    bool reorder(std::vector<int> id_order);

    bool erase(position_t pos);

    void clear_tile_layers();
    void clear_obj_layer();
    void clear_all();

    TileEntry* get_tile_layer_at(position_t pos);
    const TileEntry* get_tile_layer_at(position_t pos) const;

    inline std::vector<TileEntry>& get_tile_layers() { return tile_layers; }
    inline const std::vector<TileEntry>& get_tile_layers() const { return tile_layers; }

    inline ObjLayerT& get_obj_layer() { return obj_layer; };
    inline const ObjLayerT& get_obj_layer() const { return obj_layer; };

    inline int get_fg_count() const  { return fg_count; };
    inline int get_bg_count() const  { return bg_count; };

};


template<HasID T, HasID O>
void LevelLayerContainer<T, O>::clear_tile_layers() { tile_layers.clear(); bg_count = 0; fg_count = 0; }

template<HasID T, HasID O>
void LevelLayerContainer<T, O>::clear_obj_layer() { obj_layer.clear(); }

template<HasID T, HasID O>
void LevelLayerContainer<T, O>::clear_all() { clear_tile_layers(); clear_obj_layer(); }

template<HasID T, HasID O>
typename LevelLayerContainer<T, O>::position_t
LevelLayerContainer<T, O>::insert(position_t pos, T&& layer)
{
    if (pos == OBJECT_LAYER_POS || pos < -bg_count - 1 || pos > fg_count + 1)
    {
        throw std::out_of_range("index out of range");
    }
    if (pos < 0) {
        int ndx = (pos + 1 + bg_count);
        if (tile_layers.empty()) {
            tile_layers.emplace_back(
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
            tile_layers.emplace_back(
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

template<HasID T, HasID O>
typename LevelLayerContainer<T, O>::position_t
LevelLayerContainer<T, O>::insert(position_t pos, const T& layer) { return insert(pos, T{ layer }); }

template<HasID T, HasID O>
typename LevelLayerContainer<T, O>::position_t
LevelLayerContainer<T, O>::push_fg_front(const T& layer) { return insert(fg_count + 1, layer); }

template<HasID T, HasID O>
typename LevelLayerContainer<T, O>::position_t
LevelLayerContainer<T, O>::push_fg_front(T&& layer) { return insert(fg_count + 1, std::move(layer)); }


template<HasID T, HasID O>
typename LevelLayerContainer<T, O>::position_t
LevelLayerContainer<T, O>::push_fg_back(const T& layer) { return insert(1, layer); }

template<HasID T, HasID O>
typename LevelLayerContainer<T, O>::position_t
LevelLayerContainer<T, O>::push_fg_back(T&& layer) { return insert(1, std::move(layer)); }

template<HasID T, HasID O>
typename LevelLayerContainer<T, O>::position_t
LevelLayerContainer<T, O>::push_bg_front(const T& layer) { return insert(-1, layer); }

template<HasID T, HasID O>
typename LevelLayerContainer<T, O>::position_t
LevelLayerContainer<T, O>::push_bg_front(T&& layer) { return insert(-1, std::move(layer)); }

template<HasID T, HasID O>
typename LevelLayerContainer<T, O>::position_t
LevelLayerContainer<T, O>::push_bg_back(const T& layer) { return insert(-bg_count - 1, layer); }

template<HasID T, HasID O>
typename LevelLayerContainer<T, O>::position_t
LevelLayerContainer<T, O>::push_bg_back(T&& layer) { return insert(-bg_count - 1, std::move(layer)); }

template<HasID T, HasID O>
typename LevelLayerContainer<T, O>::position_t
LevelLayerContainer<T, O>::swap_next(position_t pos)
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

template<HasID T, HasID O>
typename LevelLayerContainer<T, O>::position_t
LevelLayerContainer<T, O>::swap_prev(position_t pos)
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

template<HasID T, HasID O>
std::vector<int> 
LevelLayerContainer<T, O>::get_id_order() const
{
    std::vector<int> ret( tile_layers.size() + 1, 0 );
    for (int i = -bg_count; i <= fg_count; i++)
    {
        if (i == 0)
            continue;

        ret.at(i + bg_count) = get_tile_layer_at(i)->tilelayer.getID();
    }
    return ret;
}

template<HasID T, HasID O>
bool 
LevelLayerContainer<T, O>::reorder(std::vector<int> id_order)
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


template<HasID T, HasID O>
bool 
LevelLayerContainer<T, O>::erase(position_t pos)
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

template<HasID T, HasID O>
typename LevelLayerContainer<T, O>::TileEntry*
LevelLayerContainer<T, O>::get_tile_layer_at(position_t pos)
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

template<HasID T, HasID O>
const typename LevelLayerContainer<T, O>::TileEntry*
LevelLayerContainer<T, O>::get_tile_layer_at(position_t pos) const
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



}