#include "fastfall/game/object/GameObject.hpp"

#include "fastfall/util/log.hpp"

/*
#include "Player.hpp"
#include "BasicPlatform.hpp"
*/

#include "fastfall/game/Instance.hpp"
#include "fastfall/game/GameObjectManager.hpp"

#include "fastfall/game/InstanceInterface.hpp"

#include <assert.h>
#include <functional>
#include <set>

namespace ff {

GameObject* GameObjectLibrary::buildFromData(GameContext instance, ObjectLevelData& data) {
	std::unique_ptr<GameObject> obj = nullptr;

	auto r = std::find_if(getBuilder().begin(), getBuilder().end(),
		[&data](const ObjectTypeBuilder& builder) {
			return data.typehash == builder.hash;
		});

	if (r != getBuilder().end()) {
		obj = r->builder(instance, r->constraints, data);

		if (obj) {
			return instance::obj_add(instance, std::move(obj));
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

		LOG_ERR_("could not match object type {}", data.typehash);
		LOG_ERR_("known types are:");
		log::scope scope;
		for (auto& type : getBuilder()) {
			LOG_ERR_("{}: {}", type.hash, type.objTypeName);
		}
	}
	return nullptr;
}

const std::string* GameObjectLibrary::lookupTypeName(size_t hash) {
	auto r = std::find_if(getBuilder().begin(), getBuilder().end(),
		[&hash](const ObjectTypeBuilder& builder) {
			return hash == builder.hash;
		});

	if (r != getBuilder().end()) {
		return &r->objTypeName;
	}
	return nullptr;
}

bool ObjectType::test(ObjectLevelData& data) const {

	if (!allow_level_data) {
		LOG_WARN("object cannot be instantiated by level {}:{:x}",
			type.name, type.hash
		);
		return false;
	}

	// test type
	if (type.hash != data.typehash) {
		LOG_WARN("object hash ({}) not valid for object {}:{:x}",
			data.typehash, type.name, type.hash
		);
		return false;
	}


	// test size
	if (tile_size.x > 0 && (data.size.x / TILESIZE != tile_size.x)) {
		LOG_WARN("object width ({}) not valid for object:{:x}",
			data.size.x, data.typehash
		);
		return false;
	}
	if (tile_size.y > 0 && (data.size.y / TILESIZE != tile_size.y)) {
		LOG_WARN("object height ({}) not valid for object:{:x}",
			data.size.y, data.typehash
		);
		return false;
	}


	// test custom properties
	for (const auto& prop : properties) {

		auto citer = data.properties.cbegin();
		for (; citer != data.properties.cend(); citer++) {
			if (citer->first == prop.name)
				break;
		}
		if (citer != data.properties.end()) {
			switch (prop.type) {
			case ObjectPropertyType::String:
				break;
			case ObjectPropertyType::Int:
				try {
					int value = std::stoi(citer->second);
				}
				catch (std::exception except) {
					LOG_WARN("object property ({}={}) not valid for object:{:x}, not convertable to int",
						citer->first, citer->second, data.typehash
					);
					return false;
				}
				break;
			case ObjectPropertyType::Float:
				try {
					float value = std::stof(citer->second);
				}
				catch (std::exception except) {
					LOG_WARN("object property ({}={}) not valid for object:{:x}, not convertable to float",
						citer->first, citer->second, data.typehash
					);
					return false;
				}
				break;
			case ObjectPropertyType::Bool:
				try {

					int value = -1;
					if (strcmp(citer->second.c_str(), "true") == 0) {
						value = 1;
					}
					else if (strcmp(citer->second.c_str(), "false") == 0) {
						value = 0;
					}
					if (value == -1)
						throw std::exception();
				}
				catch (std::exception except) {
					LOG_WARN("object property ({}={}) not valid for object:{:x}, not convertable to boolean",
						citer->first, citer->second, data.typehash
					);
					return false;
				}
				break;
			case ObjectPropertyType::Object:
				try {
					int value = std::stoi(citer->second);
					if (value < 0)
						throw std::exception();
				}
				catch (std::exception except) {
					LOG_WARN("object property ({}={}) not valid for object:{:x}, not convertable to object id",
						citer->first, citer->second, data.typehash
					);
					return false;
				}
				break;
			}
		}
		else if (!prop.default_value.empty()) {
			data.properties.insert_or_assign(prop.name, prop.default_value);
		}
		else {
			LOG_WARN("object property ({}) not defined for object:{:x}",
				prop.name, data.typehash
			);
			return false;
		}
	}
	return true;
}

GameObject::GameObject(ObjectConfig cfg)
	: m_config(cfg)
	, m_spawnID(instance::obj_reserve_spawn_id(cfg.context))
{
}


}