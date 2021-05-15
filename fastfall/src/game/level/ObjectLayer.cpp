#include "ObjectLayer.hpp"

//#include "game/Instance.hpp"

namespace ff {

ObjectLayer::ObjectLayer(/*InstanceID instance*/) :
	//objMan(instance),
	//instanceID(instance),
	layerID(0)
{
	//objMan = &Instance(instance)->getObject();
}
ObjectLayer::ObjectLayer(GameContext context, const LayerRef& layerData) :
	//objMan(instance)
	//instanceID(instance)
	layerID(layerData.id)
{
	//objMan = &Instance(instance)->getObject();
	initFromAsset(context, layerData);
}

ObjectLayer::ObjectLayer(const ObjectLayer& obj) :
	//objMan(obj.objMan),
	layerID(obj.layerID)
	//instanceID(obj.instanceID)
{
	//levelCollision = obj.levelCollision;
	ref = obj.ref;
}
ObjectLayer::ObjectLayer(ObjectLayer&& obj) noexcept :
	//objMan(obj.objMan),
	layerID(obj.layerID)
	//instanceID(obj.instanceID)
{
	//levelCollision = obj.levelCollision;
	ref = obj.ref;
}

ObjectLayer& ObjectLayer::operator=(const ObjectLayer& obj) {
	//instanceID = obj.instanceID;
	//levelCollision = obj.levelCollision;
	ref = obj.ref;
	//objMan = obj.objMan;
	return *this;
}
ObjectLayer& ObjectLayer::operator=(ObjectLayer&& obj) noexcept {
	//instanceID = obj.instanceID;
	//levelCollision = obj.levelCollision;
	ref = obj.ref;
	//objMan = obj.objMan;
	return *this;
}

void ObjectLayer::update(secs deltaTime) {

}


void ObjectLayer::initFromAsset(GameContext context, const LayerRef& layerData) {
	assert(layerData.type == LayerType::OBJECTLAYER);
	layerID = layerData.id;
	ref = &layerData;

	for (auto& objRef : layerData.objLayer.get()->objects) {
		GameObject::build(context, objRef.second);
	}
}


void ObjectLayer::clear() {

}

}