#pragma once

//#include "game/phys/collider_regiontypes/ColliderTileMap.hpp"
#include "fastfall/resource/asset/LevelAssetTypes.hpp"

//#include "util/Updatable.hpp"

#include "fastfall/game/GameObjectManager.hpp"

namespace ff {

class ObjectLayer {
public:

	ObjectLayer();
	ObjectLayer(GameContext context, unsigned id, const ObjectLayerRef& layerData);

	ObjectLayer(const ObjectLayer& obj);
	ObjectLayer(ObjectLayer&& obj) noexcept;

	ObjectLayer& operator=(const ObjectLayer& obj);
	ObjectLayer& operator=(ObjectLayer&& obj) noexcept;

	void update(secs deltaTime);

	void initFromAsset(GameContext context, unsigned id, const ObjectLayerRef& layerData);

	void clear();

	inline bool initialized() noexcept { return ref != nullptr; };

	inline unsigned int getLayerID() { return layerID; };
	inline const ObjectLayerRef* getLayerRef() { return ref; };

private:
	unsigned int layerID;
	const ObjectLayerRef* ref = nullptr;

};

}