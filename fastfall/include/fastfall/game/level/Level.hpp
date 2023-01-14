#pragma once

#include "fastfall/engine/time/time.hpp"

#include "fastfall/resource/asset/LevelAsset.hpp"
#include "fastfall/resource/ResourceSubscriber.hpp"

#include "fastfall/render/Drawable.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/game/level/LevelLayerContainer.hpp"
#include "fastfall/game/level/TileLayer.hpp"
#include "fastfall/game/level/ObjectLayer.hpp"

#include <list>
#include <concepts>

namespace ff {

class World;

class Level : public ResourceSubscriber {
public:
    struct TileLayerProxy {
        ID<TileLayer> cmp_id;
        unsigned layer_id;
        unsigned getID() const { return layer_id; }
    };

	using Layers = LevelLayerContainer<TileLayerProxy, ObjectLayer>;

	Level(World& w, ID<Level> t_id, std::optional<std::string> name, std::optional<Vec2u> size, std::optional<Color> bgColor);
	Level(World& w, ID<Level> t_id, const LevelAsset& levelData);

    void initFromAsset(World& world, const LevelAsset& levelData);

    void notifyReloadedAsset(const Asset* asset) override {
        if (asset == src_asset) {
            asset_changed = true;
        }
    }

	inline const Color& getBGColor() const { return bgColor; };
	inline const Vec2u& size() const { return levelSize; };
	inline const std::string& name() const { return levelName; };

	void resize(World& world, Vec2u n_size);
	void set_name(std::string_view name) { levelName = name; };
	void set_bg_color(Color color) { bgColor = color; };

	Layers& get_layers() { return layers; };
	const Layers& get_layers() const { return layers; };

    ObjectLayer& get_obj_layer() { return layers.get_obj_layer(); }
    const ObjectLayer& get_obj_layer() const { return layers.get_obj_layer(); }

    TileLayerProxy& get_tile_layer(unsigned id) { return layers.get_tile_layers().at(id).tilelayer; }
    const TileLayerProxy& get_tile_layer(unsigned id) const { return layers.get_tile_layers().at(id).tilelayer; }

    ID<Level> getID() const { return m_id; }

    inline bool has_src_asset_changed() const { return asset_changed; }
    bool try_reload_level(World& w);

    bool allow_asset_reload = true;

private:
    void entity_check(World& w) const;

    // asset
    const LevelAsset* src_asset = nullptr;
    bool asset_changed      = false;

    ID<Level> m_id;
	std::string levelName = "New Level";
	Color bgColor         = Color::Black;
	Vec2u levelSize       = { GAME_TILE_W, GAME_TILE_H };
	Layers layers;
};

}
