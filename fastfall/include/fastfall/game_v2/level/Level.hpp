#pragma once

#include "fastfall/engine/time/time.hpp"

#include "fastfall/resource/asset/LevelAsset.hpp"

#include "fastfall/render/Drawable.hpp"

#include "fastfall/game_v2/level/LevelLayerContainer.hpp"
#include "fastfall/game_v2/level/TileLayer.hpp"
#include "fastfall/game_v2/level/ObjectLayer.hpp"

#include <list>
#include <concepts>

namespace ff {

class World;
class LevelEditor;

class Level  {
public:
	using Layers = LevelLayerContainer<TileLayer, ObjectLayer>;

	Level(World* w);
	Level(World* w, const LevelAsset& levelData);

	void init(const LevelAsset& levelData);

	void update(secs deltaTime);

	void predraw(float interp, bool updated);

	inline const Color& getBGColor() const { return bgColor; };
	inline const Vec2u& size() const { return levelSize; };
	inline const std::string& name() const { return levelName; };

    void set_world(World* w) { world = w; }

	void resize(Vec2u n_size);
	void set_name(std::string name) { levelName = name; };
	void set_bg_color(Color color) { bgColor = color; };

	Layers& get_layers() { return layers; };
	const Layers& get_layers() const { return layers; };

	bool hasEditorHooked = false;

private:
	std::string levelName;
	Color bgColor;
	Vec2u levelSize;

	Layers layers;

    World* world;
	const LevelAsset* asset = nullptr;
};

}
