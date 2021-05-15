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

using ObjectTypeBuilder = std::function<std::unique_ptr<GameObject>(GameContext, const ObjectRef&)>;

struct ObjectType {

	template<typename T>
	struct tag { using type = T; };

	template<typename T, typename = std::enable_if<std::is_base_of<GameObject, T>::value>>
	static ObjectType create() {
		ObjectType t;
		t.objTypeName = typeid(T).name();
		t.objTypeName = t.objTypeName.substr(6); // cut off "class "

		t.hash = std::hash<std::string>{}(t.objTypeName);
		t.builder = [](GameContext inst, const ObjectRef& ref)->std::unique_ptr<GameObject>
		{
			return std::make_unique<T>(inst, ref);
		};

		return t;
	};

	ObjectType() {
		hash = 0;
	}
	ObjectType(size_t typehash) {
		hash = typehash;
	}

	size_t hash;
	std::string objTypeName;
	ObjectTypeBuilder builder;
};

struct ObjectType_compare {
	bool operator() (const ObjectType& lhs, const ObjectType& rhs) const {
		return lhs.hash < rhs.hash;
	}
};

std::set<ObjectType, ObjectType_compare> objectBuildMap{
	//ObjectType::create<Player>(),
	//ObjectType::create<BasicPlatform>()
};

template<typename T, typename>
constexpr void GameObject::addType() {
	objectBuildMap.insert(ObjectType::create<T>());
}

const std::string* GameObject::lookupTypeName(size_t hash) {
	auto r = objectBuildMap.find(hash);
	if (r != objectBuildMap.end()) {
		return &r->objTypeName;
	}
	return nullptr;
}

void GameObject::build(GameContext instance, const ObjectRef& ref) {
	std::unique_ptr<GameObject> obj = nullptr;

	auto r = objectBuildMap.find(ref.type);
	if (r != objectBuildMap.end()) {
		obj = r->builder(instance, ref);

		if (instance.valid()) {
			instance.objects().add(std::move(obj));
		}
		else {
			LOG_ERR_("No instance");
			assert(false);
		}
	}
	/*
	else {
		LOG_ERROR("could not match object type" + std::to_string(ref.type));
		assert(false);
	}
	*/
	//return obj;
}

}