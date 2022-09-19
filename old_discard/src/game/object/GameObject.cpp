#include "fastfall/game/object/GameObject.hpp"

#include "fastfall/util/log.hpp"

#include "fastfall/game/ObjectSystem.hpp"

#include "fastfall/game/InstanceInterface.hpp"

#include <assert.h>
#include <functional>
#include <set>

namespace ff {

bool ObjectType::test(ObjectLevelData& data) const {

	if (!allow_as_level_data) {
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


std::map<size_t, ObjectFactory::ObjectFactoryImpl>& ObjectFactory::getFactories() {
	static std::map<size_t, ObjectFactoryImpl> factories;
	return factories;
}

GameObject* ObjectFactory::add_obj_to_instance(GameContext cfg, std::unique_ptr<GameObject>&& obj)
{
	return instance::obj_add(cfg, std::move(obj));
}

GameObject* ObjectFactory::createFromData(GameContext cfg, ObjectLevelData& data) {
	if (auto it = getFactories().find(data.typehash); it != getFactories().end()) {

		std::unique_ptr<GameObject> obj = it->second.createfn(cfg, data);

		if (obj) {
			return instance::obj_add(cfg, std::move(obj));
		}
		else {
			LOG_ERR_("Failed to create object: {}:{}", it->second.object_type->type.name, data.level_id.id);
		}
	}
	else {
		LOG_ERR_("could not match object type {}", data.typehash);
		LOG_ERR_("known types are:");
		log::scope scope;
		for (auto& [_, impl] : getFactories()) {
			LOG_ERR_("{}: {}", impl.object_type->type.hash, impl.object_type->type.name);
		}
	}
	return nullptr;
}

const ObjectType* ObjectFactory::getType(size_t hash) {
	if (auto it = getFactories().find(hash); it != getFactories().end()) {
		return it->second.object_type;
	}
	return nullptr;
}

const ObjectType* ObjectFactory::getType(std::string_view name) {
	for (auto& [_, impl] : getFactories()) {
		if (impl.object_type->type.name == name) {
			return impl.object_type;
		}
	}
	return nullptr;
}

GameObject::GameObject(GameContext cfg)
	: m_context(cfg)
	, m_spawnID(instance::obj_reserve_spawn_id(cfg))
{
}

GameObject::GameObject(GameContext cfg, ObjectLevelData& data)
	: m_context(cfg)
	, m_spawnID(instance::obj_reserve_spawn_id(cfg))
	, m_data(&data)
{
}

}