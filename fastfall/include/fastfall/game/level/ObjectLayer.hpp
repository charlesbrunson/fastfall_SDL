#pragma once

#include "fastfall/resource/asset/LevelAssetTypes.hpp"
//#include "fastfall/game/GameContext.hpp"

namespace ff {

class ObjectLayer {
public:

	ObjectLayer();
	ObjectLayer(const ObjectLayerData& layerData);

	ObjectLayer(const ObjectLayer& obj);
	ObjectLayer(ObjectLayer&& obj) noexcept;

	ObjectLayer& operator=(const ObjectLayer& obj);
	ObjectLayer& operator=(ObjectLayer&& obj) noexcept;

	void initFromAsset(const ObjectLayerData& layerData);

	void clear();

	void createObjectsFromData();

	inline unsigned int getID() const { return layerID; };
	inline const std::vector<ObjectLevelData>& getObjectData() { return object_refs; };

	const ObjectLevelData* getObjectDataByID(ObjLevelID id) const;

	void addObjectData(ObjectData ref);
	bool removeObjectDataByID(ObjLevelID id);

private:
	unsigned int layerID;
	std::vector<ObjectLevelData> object_refs;

	unsigned lastID = 1;
};

}