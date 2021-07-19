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

void GameObjectLibrary::build(GameContext instance, const ObjectRef& ref) {
	std::unique_ptr<GameObject> obj = nullptr;

	auto r = getBuilder().find(ref.type);
	if (r != getBuilder().end()) {
		obj = r->builder(instance, ref, r->constraints);

		if (obj && instance.valid()) {
			instance.objects().add(std::move(obj));
		}
		else if (!obj) {
			LOG_ERR_("Object reference invalid, unable to create");
			//assert(false);
		}
		else {
			LOG_ERR_("No instance");
			assert(false);
		}
	}
	else {

		LOG_ERR_("could not match object type {}", ref.type);
		LOG_ERR_("known types are:");
		log::scope scope;
		for (auto& type : getBuilder()) {
			LOG_ERR_("{}: {}", type.hash, type.objTypeName);
		}
	}
}

const std::string* GameObjectLibrary::lookupTypeName(size_t hash) {
	auto r = getBuilder().find(hash);
	if (r != getBuilder().end()) {
		return &r->objTypeName;
	}
	return nullptr;
}

bool ObjectType::test(ObjectRef& ref) const {
	bool valid = true;


	// test size
	if (tile_size.x > 0 && (ref.width / TILESIZE != tile_size.x)) {
		LOG_WARN("object width ({}) not valid for object:{}",
			ref.width, ref.id
		);
		valid = false;
	}
	if (tile_size.y > 0 && (ref.height / TILESIZE != tile_size.y)) {
		LOG_WARN("object height ({}) not valid for object:{}",
			ref.width, ref.id
		);
		valid = false;
	}


	// test custom properties
	for (const auto& prop : properties) {

		auto citer = ref.properties.cbegin();
		for (; citer != ref.properties.cend(); citer++) {
			if (citer->first == prop.name)
				break;
		}
		if (citer != ref.properties.end()) {
			switch (prop.type) {
			case ObjectPropertyType::String:
				break;
			case ObjectPropertyType::Int:
				try {
					int value = std::stoi(citer->second);
				}
				catch (std::exception except) {
					LOG_WARN("object property ({}={}) not valid for object:{}, not convertable to int",
						citer->first, citer->second, ref.id
					);
					valid = false;
				}
				break;
			case ObjectPropertyType::Float:
				try {
					float value = std::stof(citer->second);
				}
				catch (std::exception except) {
					LOG_WARN("object property ({}={}) not valid for object:{}, not convertable to float",
						citer->first, citer->second, ref.id
					);
					valid = false;
				}
				break;
			case ObjectPropertyType::Bool:
				try {
					int value = std::stoi(citer->second);
					if (value != 0 && value != 1)
						throw std::exception();
				}
				catch (std::exception except) {
					LOG_WARN("object property ({}={}) not valid for object:{}, not convertable to boolean",
						citer->first, citer->second, ref.id
					);
					valid = false;
				}
				break;
			case ObjectPropertyType::Object:
				try {
					int value = std::stoi(citer->second);
					if (value < 0)
						throw std::exception();
				}
				catch (std::exception except) {
					LOG_WARN("object property ({}={}) not valid for object:{}, not convertable to object id",
						citer->first, citer->second, ref.id
					);
					valid = false;
				}
				break;
			}
		}
		else if (!prop.default_value.empty()) {
			ref.properties.push_back({ prop.name, prop.default_value });
		}
		else {
			LOG_WARN("object property ({}) not defined for object:{}",
				prop.name, ref.id
			);
			valid = false;
		}
	}
	return valid;
}




}