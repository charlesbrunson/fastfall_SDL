#pragma once

//#include "util/Updatable.hpp"
#include "fastfall/engine/time/time.hpp"
#include "level/LevelLayerContainer.hpp"

#include "fastfall/resource/asset/LevelAsset.hpp"

#include "fastfall/render/Drawable.hpp"

#include "fastfall/game/level/TileLayer.hpp"
#include "fastfall/game/level/ObjectLayer.hpp"

#include <list>
#include <concepts>

namespace ff {

class LevelEditor;

class Level  {
public:
	using Layers = LevelLayerContainer<TileLayer, ObjectLayer>;

	Level(GameContext context);
	Level(GameContext context, const LevelAsset& levelData);

	void init(const LevelAsset& levelData);

	void update(secs deltaTime);

	void predraw(secs deltaTime);

	inline const Color& getBGColor() const { return bgColor; };
	inline const Vec2u& size() const { return levelSize; };
	inline const std::string& name() const { return levelName; };

	GameContext getContext() { return context; }
	inline InstanceID getInstanceID() { return context.getID(); };

	void resize(Vec2u n_size);
	void set_name(std::string name) { levelName = name; };
	void set_bg_color(Color color) { bgColor = color; };

	Layers& get_layers() { return layers; };
	const Layers& get_layers() const { return layers; };

	bool hasEditorHooked = false;

private:
	GameContext context;

	std::string levelName;
	Color bgColor;
	Vec2u levelSize;

	Layers layers;

};

}
