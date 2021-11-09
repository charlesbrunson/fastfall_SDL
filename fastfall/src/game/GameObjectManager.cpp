#include "fastfall/game/GameObjectManager.hpp"

#include <assert.h>

namespace ff {

GameObjectManager::GameObjectManager(unsigned instance) :
	instanceID(instance)
{

}

GameObjectManager::GameObjectManager(const GameObjectManager& obj) :
	instanceID(obj.instanceID)
{
	for (auto& objptr : obj.objects) {
		addObject(objptr->clone());
	}
}
GameObjectManager::GameObjectManager(GameObjectManager&& obj) noexcept :
	instanceID(obj.instanceID)
{
	std::swap(objects, obj.objects);
}

GameObjectManager& GameObjectManager::operator=(const GameObjectManager& obj) {
	instanceID = obj.instanceID;

	clear();
	for (auto& objptr : obj.objects) {
		addObject(objptr->clone());
	}

	return *this;
}

GameObjectManager& GameObjectManager::operator=(GameObjectManager&& obj) noexcept {
	instanceID = obj.instanceID;
	std::swap(objects, obj.objects);
	return *this;
}

void GameObjectManager::update(secs deltaTime) {
	//if (deltaTime <= 0.0)
	//	return;

	for (auto& obj : objects) {
		obj->update(deltaTime);
	}
}
void GameObjectManager::predraw(secs deltaTime) {
	for (auto& obj : objects) {
		obj->predraw(deltaTime);
	}
}

void GameObjectManager::clear() {
	objects.clear();
}

void GameObjectManager::addObject(std::unique_ptr<GameObject>&& obj) {
	assert(obj);
	if (!obj->hasCollider) {
		objects.push_back(std::move(obj));
	}
	else {
		objects.insert(std::begin(objects), std::move(obj));
	}
}

}