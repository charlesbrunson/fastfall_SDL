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

	void initFromAsset(GameContext context, unsigned id, const ObjectLayerRef& layerData);

	void clear();

	void createObjects(GameContext context);

	inline unsigned int getLayerID() { return layerID; };
	inline const std::vector<ObjectRef>& getObjectRefs() { return object_refs; };

	const ObjectRef* getRefByID(unsigned obj_id) const;

	void addObjectRef(ObjectRef ref);
	bool removeObjectRef(object_id id);

private:
	unsigned int layerID;
	std::vector<ObjectRef> object_refs;

};

}