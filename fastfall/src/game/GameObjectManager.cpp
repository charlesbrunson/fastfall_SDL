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
	std::swap(drawOrderBG, obj.drawOrderBG);
	std::swap(drawOrderFG, obj.drawOrderFG);
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
	std::swap(drawOrderBG, obj.drawOrderBG);
	std::swap(drawOrderFG, obj.drawOrderFG);
	return *this;
}

void GameObjectManager::update(secs deltaTime) {
	if (deltaTime <= 0.0)
		return;

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
	drawOrderFG.drawOrder.clear();
	drawOrderBG.drawOrder.clear();
}

void GameObjectManager::addObject(std::unique_ptr<GameObject>&& obj) {

	assert(obj);

	GameObject* object;

	if (!obj->hasCollider) {
		objects.push_back(std::move(obj));
		object = objects.back().get();
	}
	else {
		objects.push_front(std::move(obj));
		object = objects.front().get();
	}

	int priority = object->getDrawPriority();

	(priority <= 0 ? drawOrderBG : drawOrderFG)
		.drawOrder.insert(std::make_pair(priority, object));
}

void GameObjectManager::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(drawOrderBG, states);
}

}