#pragma once

#include "fastfall/engine/time/time.hpp"

#include "fastfall/resource/asset/LevelAsset.hpp"

#include "fastfall/render/Drawable.hpp"

#include "fastfall/game/level/LevelLayerContainer.hpp"
#include "fastfall/game/level/TileLayer.hpp"
#include "fastfall/game/level/ObjectLayer.hpp"

#include <list>
#include <concepts>

namespace ff {

class World;
class LevelEditor;

class Level  {
public:
	using Layers = LevelLayerContainer<TileLayer, ObjectLayer>;

	Level(World& w, ID<Level> t_id);
	Level(World& w, ID<Level> t_id, const LevelAsset& levelData);


    //void init(World& world, ID<Level> t_id);
    void initFromAsset(World& world, const LevelAsset& levelData);

    // removes all layers, resets name, bg color and size
    void clean(World& w);

	void update(World& world, secs deltaTime);
	void predraw(World& world, float interp, bool updated);

	inline const Color& getBGColor() const { return bgColor; };
	inline const Vec2u& size() const { return levelSize; };
	inline const std::string& name() const { return levelName; };

	void resize(World& world, Vec2u n_size);
	void set_name(std::string name) { levelName = name; };
	void set_bg_color(Color color) { bgColor = color; };

	Layers& get_layers() { return layers; };
	const Layers& get_layers() const { return layers; };

    ObjectLayer& get_obj_layer() { return layers.get_obj_layer(); }
    const ObjectLayer& get_obj_layer() const { return layers.get_obj_layer(); }

    TileLayer& get_tile_layer(unsigned id) { return layers.get_tile_layers().at(id).tilelayer; }
    const TileLayer& get_tile_layer(unsigned id) const { return layers.get_tile_layers().at(id).tilelayer; }

	bool hasEditorHooked = false;

    ID<Level> getID() const { return m_id; }

private:
    ID<Level> m_id;
	std::string levelName;
	Color bgColor;
	Vec2u levelSize;
	Layers layers;
};

}
