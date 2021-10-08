#pragma once

//#include "game/phys/collider_regiontypes/ColliderTileMap.hpp"
#include "fastfall/resource/asset/LevelAssetTypes.hpp"

//#include "util/Updatable.hpp"

#include "fastfall/game/GameObjectManager.hpp"

namespace ff {

class ObjectLayer {
public:

	ObjectLayer();
	ObjectLayer(GameContext context, const ObjectLayerData& layerData);

	ObjectLayer(const ObjectLayer& obj);
	ObjectLayer(ObjectLayer&& obj) noexcept;

	ObjectLayer& operator=(const ObjectLayer& obj);
	ObjectLayer& operator=(ObjectLayer&& obj) noexcept;

	void initFromAsset(GameContext context, const ObjectLayerData& layerData);

	void clear();

	void createObjects(GameContext context);

	inline unsigned int getID() const { return layerID; };
	inline const std::vector<ObjectData>& getObjectRefs() { return object_refs; };

	const ObjectData* getRefByID(unsigned obj_id) const;

	void addObjectRef(ObjectData ref);
	bool removeObjectRef(object_id id);

private:
	unsigned int layerID;
	std::vector<ObjectData> object_refs;

};

}