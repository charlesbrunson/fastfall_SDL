#include "fastfall/game/object/Object.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/engine/config.hpp"
#include "fastfall/game/World.hpp"

#include <assert.h>
#include <functional>
#include <set>

namespace ff {

bool ObjectType::test(LevelObjectData& data) const {

    /*
	if (!allow_as_level_data) {
		LOG_WARN("object cannot be instantiated by level {}:{:x}",
			type.name, type.hash
		);
		return false;
	}
    */

	// test type
	if (name.hash != data.typehash) {
		LOG_WARN("object hash ({}) not valid for object {}:{:x}",
			data.typehash, name.str, name.hash
		);
		return false;
	}

	// test size
	if (tile_size.x > 0 && (data.area.width / TILESIZE != tile_size.x)) {
		LOG_WARN("object width ({}) not valid for object:{:x}",
			data.area.width, data.typehash
		);
		return false;
	}
	if (tile_size.y > 0 && (data.area.height / TILESIZE != tile_size.y)) {
		LOG_WARN("object height ({}) not valid for object:{:x}",
			data.area.height, data.typehash
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


std::unordered_map<size_t, ObjectFactory::ObjectBuilder> ObjectFactory::object_builders;

copyable_unique_ptr<Actor> ObjectFactory::createFromData(ActorInit init, LevelObjectData& data) {
	if (auto it = object_builders.find(data.typehash); it != object_builders.end())
    {
        auto& builder = it->second;
        init.type     = ActorType::Object;
        init.priority = builder.type->priority;
        copyable_unique_ptr<Actor> obj = builder.create(init, data);
		if (obj) {
			return std::move(obj);
		}
		else {
			LOG_ERR_("Failed to create object: {}:{}", data.name, data.level_id.id);
		}
	}
	else
    {
		LOG_ERR_("could not match object type {}", data.typehash);
		LOG_ERR_("known types are:");
		log::scope scope;
		for (auto& [_, impl] : object_builders) {
			LOG_ERR_("{}: {}", impl.type->name.hash, impl.type->name.str);
		}
	}
	return copyable_unique_ptr<Actor>{};
}

const ObjectType* ObjectFactory::getType(size_t hash) {
	if (auto it = object_builders.find(hash); it != object_builders.end()) {
		return it->second.type;
	}
	return nullptr;
}

const ObjectType* ObjectFactory::getType(std::string_view name) {
    size_t hash = std::hash<std::string_view>{}(name);
	for (auto& [_, impl] : object_builders) {
		if (impl.type->name.hash == hash) {
			return impl.type;
		}
	}
	return nullptr;
}

Object::Object(ActorInit init, const ObjectType& type, const LevelObjectData* data)
    : Actor( init, type.name.str )
    , obj_type(&type)
	, obj_data(data)
{
}

}