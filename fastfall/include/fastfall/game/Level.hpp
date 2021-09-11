#pragma once

//#include "util/Updatable.hpp"
#include "fastfall/engine/time/time.hpp"
#include "level/TileLayer.hpp"
#include "level/ObjectLayer.hpp"

#include "fastfall/resource/asset/LevelAsset.hpp"

#include "fastfall/render/Drawable.hpp"

#include <list>

namespace ff {

class Level  {
public:

	Level(GameContext context);
	Level(GameContext context, const LevelAsset& levelData);

	void init(const LevelAsset& levelData);

	void update(secs deltaTime);

	void predraw(secs deltaTime);

	inline const Color& getBGColor() const { return bgColor; };
	inline const Vec2u& size() const { return levelSize; };
	inline const std::string& name() const { return levelName; };


	inline std::vector<TileLayer>& getBGLayers() { return bgLayers; };
	inline std::vector<TileLayer>& getFGLayers() { return fgLayers; };

	const inline std::vector<TileLayer>& getBGLayers() const { return bgLayers; };
	const inline std::vector<TileLayer>& getFGLayers() const { return fgLayers; };

	inline ObjectLayer& getObjLayer() { return objLayer; };
	const inline ObjectLayer& getObjLayer() const { return objLayer; };

	inline InstanceID getInstanceID() { return context.getID(); };

private:
	GameContext context;

	std::string levelName;
	Color bgColor;
	Vec2u levelSize;
	unsigned bordersCardinalBits;

	std::vector<TileLayer> bgLayers;
	ObjectLayer objLayer;
	std::vector<TileLayer> fgLayers;

};

}