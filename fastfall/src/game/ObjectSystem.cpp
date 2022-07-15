#include "fastfall/game/ObjectSystem.hpp"

/*

#include <assert.h>

namespace ff {

ObjectSystem::ObjectSystem()
{

}

ObjectSystem::ObjectSystem(const ObjectSystem& obj)
{
	for (auto& objptr : obj.objects) {
		addObject(objptr->clone());
	}
}
ObjectSystem::ObjectSystem(ObjectSystem&& obj) noexcept
{
	std::swap(objects, obj.objects);
}

ObjectSystem& ObjectSystem::operator=(const ObjectSystem& obj) {

	clear();
	for (auto& objptr : obj.objects) {
		addObject(objptr->clone());
	}

	return *this;
}

ObjectSystem& ObjectSystem::operator=(ObjectSystem&& obj) noexcept {
	std::swap(objects, obj.objects);
	return *this;
}

void ObjectSystem::update(secs deltaTime)
{
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
void ObjectSystem::predraw(float interp, bool updated) {

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

void ObjectSystem::clear() {
	objects.clear();
	spawnCounter = 1;
}

void ObjectSystem::addObject(std::unique_ptr<GameObject>&& obj) {
	assert(obj);
	created_objects.push_back(std::move(obj));
}

}

*/