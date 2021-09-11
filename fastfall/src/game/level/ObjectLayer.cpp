#include "fastfall/game/level/ObjectLayer.hpp"

namespace ff {

ObjectLayer::ObjectLayer()
	: layerID(0)
{

}
ObjectLayer::ObjectLayer(GameContext context, unsigned id, const ObjectLayerRef& layerData)
	: layerID(id)
{
	initFromAsset(context, id, layerData);
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

void ObjectLayer::initFromAsset(GameContext context, unsigned id, const ObjectLayerRef& layerData)
{
	layerID = id;
	object_refs = layerData.objects;
}


const ObjectRef* ObjectLayer::getRefByID(unsigned obj_id) const {
	const ObjectRef* ref = nullptr;

	if (obj_id != object_null) {
		auto it = std::find_if(
			object_refs.begin(), object_refs.end(),
			[obj_id](const ObjectRef& ref) {
				return ref.id == obj_id;
			});

		if (it != object_refs.end()) {
			ref = &*it;
		}
	}
	return ref;
}

void ObjectLayer::addObjectRef(ObjectRef ref) {
	if (object_refs.empty()) {
		ref.id = 1;
	}
	else {
		ref.id = object_refs.back().id + 1;
	}
	object_refs.push_back(ref);
}

bool ObjectLayer::removeObjectRef(object_id id) {

	if (id != object_null) {
		auto it = std::find_if(
			object_refs.begin(), object_refs.end(),
			[id](const ObjectRef& ref) {
				return ref.id == id;
			});

		if (it != object_refs.end()) {
			object_refs.erase(it);
			return true;
		}
	}
	return false;
}

void ObjectLayer::createObjects(GameContext context) {
	for (auto& objRef : object_refs) {
		if (objRef.type != 0) {
			GameObjectLibrary::build(context, objRef);
		}
	}
}

void ObjectLayer::clear() {
	object_refs.clear();
}

}