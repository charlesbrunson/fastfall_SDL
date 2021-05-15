#pragma once

//#include "util/Updatable.hpp"
#include "fastfall/engine/time/time.hpp"
#include "level/TileLayer.hpp"
#include "level/ObjectLayer.hpp"

#include "fastfall/resource/asset/LevelAsset.hpp"

#include "fastfall/render/Drawable.hpp"

#include <list>

namespace ff {

class Level : public Drawable {
public:

	Level(GameContext context);
	Level(GameContext context, const LevelAsset& levelData);
	~Level();

	void init(const LevelAsset& levelData);

	void update(secs deltaTime);

	void predraw(secs deltaTime);

	inline const Color& getBGColor() const { return bgColor; };
	inline const Vec2u& size() const { return levelSize; };
	inline const std::string* name() const { return levelName; };


	inline std::list<TileLayer>& getBGLayers() { return bgLayers; };
	inline std::list<TileLayer>& getFGLayers() { return fgLayers; };
	inline ObjectLayer& getObjLayer() { return objLayer; };

	inline InstanceID getInstanceID() { return context.getID(); };

private:
	//static unsigned instanceCounter;

	GameContext context;

	void draw(RenderTarget& target, RenderState states = RenderState()) const override;

	const std::string* levelName;
	Vec2u levelSize;
	unsigned bordersCardinalBits;

	Color bgColor;

	std::list<TileLayer> bgLayers;
	ObjectLayer objLayer;
	std::list<TileLayer> fgLayers;

};

}