#include "fastfall/game/level/ObjectLayer.hpp"

#include "fastfall/game/ObjectSystem.hpp"
#include "fastfall/game/World.hpp"

namespace ff {

ObjectLayer::ObjectLayer()
	: layerID(0)
{

}
ObjectLayer::ObjectLayer(const ObjectLayerData& layerData)
{
	initFromAsset(layerData);
}

ObjectLayer::ObjectLayer(const ObjectLayer& obj)
	: layerID(obj.layerID)
{
	object_refs = obj.object_refs;
}
ObjectLayer::ObjectLayer(ObjectLayer&& obj) noexcept 
	: layerID(obj.layerID)
{
	object_refs = std::move(obj.object_refs);
}

ObjectLayer& ObjectLayer::operator=(const ObjectLayer& obj) 
{
	object_refs = obj.object_refs;
	return *this;
}
ObjectLayer& ObjectLayer::operator=(ObjectLayer&& obj) noexcept 
{
	object_refs = std::move(obj.object_refs);
	return *this;
}

void ObjectLayer::initFromAsset(const ObjectLayerData& layerData)
{
	layerID = layerData.getID();
	object_refs = layerData.objects;
}

const ObjectLevelData* ObjectLayer::getObjectDataByID(ObjLevelID obj_id) const {
	const ObjectLevelData* ref = nullptr;

	if (obj_id) {
		auto it = std::find_if(
			object_refs.begin(), object_refs.end(),
			[obj_id](const ObjectLevelData& ref) {
				return ref.level_id == obj_id;
			});

		if (it != object_refs.end()) {
			ref = &*it;
		}
	}
	return ref;
}

void ObjectLayer::addObjectData(ObjectData ref) {
	unsigned id;

	if (object_refs.empty()) {
		id = 1;
	}
	else {
		id = object_refs.back().level_id.id + 1;
	}

	ObjectLevelData lvlref{ ref };
	lvlref.level_id = ObjLevelID{ id };

	object_refs.push_back(lvlref);
}

bool ObjectLayer::removeObjectDataByID(ObjLevelID id) {

	if (id) {
		auto it = std::find_if(
			object_refs.begin(), object_refs.end(),
			[id](const ObjectLevelData& ref) {
				return ref.level_id == id;
			});

		if (it != object_refs.end()) {
			object_refs.erase(it);
			return true;
		}
	}
	return false;
}

void ObjectLayer::createObjectsFromData(World& world) {
	for (auto& objRef : object_refs) {
		if (objRef.typehash != 0) {
            objRef.all_objects = &object_refs;
			world.add_object(ObjectFactory::createFromData(world, objRef));
		}
	}
}

void ObjectLayer::clear() {
	object_refs.clear();
}

}