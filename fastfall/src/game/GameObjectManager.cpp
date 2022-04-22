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

	for (auto& obj : created_objects) {
		if (!obj->m_has_collider) {
			objects.push_back(std::move(obj));
		}
		else {
			objects.insert(std::begin(objects), std::move(obj));
		}
	}
	created_objects.clear();
}
void GameObjectManager::predraw(float interp, bool updated) {

	auto it = std::remove_if(
		std::begin(objects),
		std::end(objects),
		[](auto& obj) {
			return obj->can_remove();
		});

	objects.erase(it, std::end(objects));

	for (auto& obj : objects) {
		obj->predraw(interp, updated);
	}
}

void GameObjectManager::clear() {
	objects.clear();
	spawnCounter = 1;
}

void GameObjectManager::addObject(std::unique_ptr<GameObject>&& obj) {
	assert(obj);
	created_objects.push_back(std::move(obj));
}

}
