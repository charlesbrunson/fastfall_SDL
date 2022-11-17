#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/Vec2.hpp"
#include "fastfall/util/log.hpp"

#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include <variant>
#include <string_view>

namespace ff {

static constexpr unsigned GID_INVALID = UINT32_MAX;
using gid = uint32_t;

// map of the first gid mapped to the tileset name
using TilesetMap = std::map<gid, std::string>;

struct ObjLevelID {
	static constexpr unsigned NO_ID = 0;

	unsigned id = NO_ID;

	bool operator== (const ObjLevelID& rhs) const {
		return id == rhs.id;
	}
	bool operator< (const ObjLevelID& rhs) const {
		return id < rhs.id;
	}

	operator bool() {
		return id != NO_ID;
	}
};

struct ObjectData {
	std::string name;
	size_t typehash = 0; // hash of type string
	Vec2i position;
	Vec2u size;
	std::unordered_map<std::string, std::string> properties;
	std::vector<Vec2i> points;

    Vec2f getPosition() const {
        return Vec2f{ position };
    }

    Vec2f getTopLeftPos() const {
        return Vec2f{ position } - Vec2f{ (float)size.x / 2.f, (float)size.y };
    }

    bool hasProp(const std::string& key) const {
        return properties.contains(key);
    }

	const std::string& getPropAsString(const std::string& key) const
	{
		return properties.at(key);
	}

    std::optional<std::string> optPropAsString(const std::string& key) const {
        return properties.contains(key) ? std::make_optional(getPropAsString(key)) : std::nullopt;
    }

	int getPropAsInt(const std::string& key) const
	{
		return std::atoi(properties.at(key).c_str());
	}

    std::optional<int> optPropAsInt(const std::string& key) const {
        return properties.contains(key) ? std::make_optional(getPropAsInt(key)) : std::nullopt;
    }

	bool getPropAsBool(const std::string& key) const
	{
		const std::string& value = properties.at(key);
		if (strcmp(value.c_str(), "true") == 0) {
			return true;
		}
		else if (strcmp(value.c_str(), "false") == 0) {
			return false;
		}
		else {
			throw std::exception();
		}
	}

    std::optional<bool> optPropAsBool(const std::string& key) const {
        return properties.contains(key) ? std::make_optional(getPropAsBool(key)) : std::nullopt;
    }

	float getPropAsFloat(const std::string& key) const
	{
		return std::atof(properties.at(key).c_str());
	}

    std::optional<float> optPropAsFloat(const std::string& key) const {
        return properties.contains(key) ? std::make_optional(getPropAsFloat(key)) : std::nullopt;
    }

	ObjLevelID getPropAsID(const std::string& key) const {
		return ObjLevelID{ (unsigned)std::atol(properties.at(key).c_str()) };
	}

    std::optional<ObjLevelID> optPropAsID(const std::string& key) const {
        return properties.contains(key) ? std::make_optional(getPropAsID(key)) : std::nullopt;
    }
};

struct ObjectLevelData : public ObjectData {
	ObjLevelID level_id;
    const std::vector<ObjectLevelData>* all_objects = nullptr;

	bool operator< (const ObjectLevelData& rhs) const {
		return level_id < rhs.level_id;
	}

    const ObjectLevelData* get_sibling(ObjLevelID obj_id) const {
        const ObjectLevelData* ref = nullptr;

        if (obj_id && all_objects) {
            auto it = std::find_if(
                    all_objects->begin(), all_objects->end(),
                    [obj_id](const ObjectLevelData& ref) {
                        return ref.level_id == obj_id;
                    });

            if (it != all_objects->end()) {
                ref = &*it;
            }
        }
        return ref;
    }
};

struct ObjectLayerData {
	unsigned layer_id = 0;
	std::string layer_name;

	std::vector<ObjectLevelData> objects;

	unsigned getID() const { return layer_id; }
};

}
