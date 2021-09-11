#include "fastfall/game/level/ObjectLayer.hpp"

namespace ff {

ObjectLayer::ObjectLayer()
	: layerID(0)
{
	//objMan = &Instance(instance)->getObject();
}
ObjectLayer::ObjectLayer(GameContext context, unsigned id, const ObjectLayerRef& layerData)
	: layerID(id)
{
	initFromAsset(context, id, layerData);
}

ObjectLayer::ObjectLayer(const ObjectLayer& obj)
	: layerID(obj.layerID)
{
	ref = obj.ref;
}
ObjectLayer::ObjectLayer(ObjectLayer&& obj) noexcept 
	: layerID(obj.layerID)
{
	ref = obj.ref;
}

ObjectLayer& ObjectLayer::operator=(const ObjectLayer& obj) 
{
	ref = obj.ref;
	return *this;
}
ObjectLayer& ObjectLayer::operator=(ObjectLayer&& obj) noexcept {
	ref = obj.ref;
	return *this;
}

void ObjectLayer::update(secs deltaTime) {

}


void ObjectLayer::initFromAsset(GameContext context, unsigned id, const ObjectLayerRef& layerData)
{
	layerID = id;
	ref = &layerData;

	for (auto& objRef : layerData.objects) {
		if (objRef.type != 0) {
			GameObjectLibrary::build(context, objRef);
		}
	}
}


void ObjectLayer::clear() {

}

}