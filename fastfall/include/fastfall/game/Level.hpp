#pragma once

//#include "util/Updatable.hpp"
#include "fastfall/engine/time/time.hpp"
#include "level/TileLayer.hpp"
#include "level/ObjectLayer.hpp"

#include "fastfall/resource/asset/LevelAsset.hpp"

#include "fastfall/render/Drawable.hpp"

#include <list>
#include <concepts>

namespace ff {

struct LevelLayer {

	//-2 = second background layer (first draw)
	//-1 = first background layer  (second draw)
	// 0 = first foreground layer  (third draw)
	// etc
	int position;

	TileLayer tilelayer;
};

class Level  {
private:

public:
	Level(GameContext context);
	Level(GameContext context, const LevelAsset& levelData);

	void init(const LevelAsset& levelData);

	void update(secs deltaTime);

	void predraw(secs deltaTime);

	inline const Color& getBGColor() const { return bgColor; };
	inline const Vec2u& size() const { return levelSize; };
	inline const std::string& name() const { return levelName; };

	std::vector<LevelLayer>& getTileLayers() { return layers; };
	const std::vector<LevelLayer>& getTileLayers() const { return layers; };
	int getFGStartNdx() const { return fg1_layer_ndx; };

	inline ObjectLayer& getObjLayer() { return objLayer; };
	const inline ObjectLayer& getObjLayer() const { return objLayer; };

	GameContext getContext() { return context; }
	inline InstanceID getInstanceID() { return context.getID(); };



	void resize(Vec2u n_size);
	void set_name(std::string name) { levelName = name; };
	void set_bg_color(Color color) { bgColor = color; };

	LevelLayer& insertTileLayer(LevelLayer&& layer);
	void removeTileLayer(int position);

	bool hasEditorHooked = false;

private:
	GameContext context;

	std::string levelName;
	Color bgColor;
	Vec2u levelSize;

	int fg1_layer_ndx = 0;
	std::vector<LevelLayer> layers;

	ObjectLayer objLayer;

};

}
