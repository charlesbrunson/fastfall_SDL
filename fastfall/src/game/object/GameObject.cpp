#include "fastfall/game/object/GameObject.hpp"

#include "fastfall/util/log.hpp"

/*
#include "Player.hpp"
#include "BasicPlatform.hpp"
*/

#include "fastfall/game/Instance.hpp"
#include "fastfall/game/GameObjectManager.hpp"

#include <assert.h>
#include <functional>
#include <set>

namespace ff {


std::unique_ptr<std::set<GameObjectLibrary::ObjectType, GameObjectLibrary::ObjectType_compare>> GameObjectLibrary::objectBuildMap;


void GameObjectLibrary::build(GameContext instance, const ObjectRef& ref) {
	std::unique_ptr<GameObject> obj = nullptr;

	auto r = getBuilder().find(ref.type);
	if (r != getBuilder().end()) {
		obj = r->builder(instance, ref);

		if (instance.valid()) {
			instance.objects().add(std::move(obj));
		}
		else {
			LOG_ERR_("No instance");
			assert(false);
		}
	}
	else {

		LOG_ERR_("could not match object type {}", ref.type);
	}
}

const std::string* GameObjectLibrary::lookupTypeName(size_t hash) {
	auto r = getBuilder().find(hash);
	if (r != getBuilder().end()) {
		return &r->objTypeName;
	}
	return nullptr;
}

}