#pragma once

//#include "game/phys/collider_regiontypes/ColliderTileMap.hpp"
#include "fastfall/resource/asset/LevelAssetTypes.hpp"

//#include "util/Updatable.hpp"

#include "fastfall/game/GameObjectManager.hpp"

namespace ff {

class ObjectLayer {
public:

	ObjectLayer(/*InstanceID instance*/);
	ObjectLayer(GameContext context, const LayerRef& layerData);

	ObjectLayer(const ObjectLayer& obj);
	ObjectLayer(ObjectLayer&& obj) noexcept;

	ObjectLayer& operator=(const ObjectLayer& obj);
	ObjectLayer& operator=(ObjectLayer&& obj) noexcept;

	void update(secs deltaTime);

	//void predraw(secs deltaTime) override;

	void initFromAsset(GameContext context, const LayerRef& layerData);

	void clear();

	//GameObject* addObject(std::unique_ptr<GameObject>&& obj);
	//GameObject* getObjectByID(unsigned int id);

	//CollisionMap* levelCollision = nullptr;

	inline bool initialized() noexcept { return ref != nullptr; };

	inline unsigned int getLayerID() { return layerID; };
	inline const LayerRef* getLayerRef() { return ref; };

	/*
	inline GameObjectManager* getObjMan() {
		return objMan;
	}
	inline const GameObjectManager* getObjMan() const {
		return objMan;
	}
	*/


private:
	//InstanceID instanceID;

	unsigned int layerID;

	//void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates()) const override;

	const LayerRef* ref = nullptr;

	//GameObjectManager* objMan = nullptr;

};

}