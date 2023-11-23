#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/Vec2.hpp"
#include "fastfall/util/log.hpp"
#include "fastfall/render/util/Color.hpp"

#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include <variant>
#include <string_view>
#include <filesystem>

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

struct ObjectProperty
{
    using Variant = std::variant<
        bool,
        Color,
        float,
        std::filesystem::path,
        int,
        ObjLevelID,
        std::string
    >;

    enum class Type {
        Bool   = 0,
        Color  = 1,
        Float  = 2,
        File   = 3,
        Int    = 4,
        Object = 5,
        String = 6
    };

    constexpr inline static std::string_view string[] = {
        "bool",
        "color",
        "float",
        "file",
        "int",
        "object",
        "string",
    };

    Variant value;
    std::string str_value = "";

    Type get_type() const { return static_cast<Type>(value.index()); }
    std::string_view get_type_str() const { return string[ value.index() ]; }
};

struct ObjectData {
	std::string name;
    std::string type;
	size_t      typehash = 0; // hash of type string
    Rectf       area;
	std::map<std::string, ObjectProperty, std::less<>> properties;
	std::vector<Vec2i> points;

    template<typename T>
    [[nodiscard]] T get_prop(std::string_view name) const {
        auto it = properties.find(name);
        bool valid = it != properties.end()
                && (it->second.get_type() != ObjectProperty::Type::String || it->second.str_value != "");
        if (it != properties.end()) {
            if (std::holds_alternative<T>(it->second.value)) {
                return std::get<T>(it->second.value);
            }
            else {
                LOG_WARN("Object property {} does not contains expected type, instead holds {}", name, it->second.get_type_str());
            }
        }
        else {
            LOG_WARN("Object property {} not found", name);
        }
        return T{};
    }

    template<typename T>
    [[nodiscard]] std::optional<T> get_prop_opt(std::string_view name) const {
        auto it = properties.find(name);
        if (it != properties.end()) {
            if (std::holds_alternative<T>(it->second.value)) {
                return std::get<T>(it->second.value);
            }
            else {
                LOG_WARN("Object property {} does not contains expected type, instead holds {}", name, it->second.get_type_str());
            }
        }
        return std::nullopt;
    }

    template<typename T>
    [[nodiscard]] bool has_prop(std::string_view name) const {
        auto it = properties.find(name);
        return it != properties.end() && std::holds_alternative<T>(it->second.value);
    }

    template<typename T>
    [[nodiscard]] T get_prop_or(std::string_view name, T alt_value) const {
        auto it = properties.find(name);
        if (it != properties.end() && std::holds_alternative<T>(it->second.value)) {
            return std::get<T>(it->second.value);
        }
        else {
            return alt_value;
        }
    }
};

struct LevelObjectData : public ObjectData {
	ObjLevelID level_id;
    const std::vector<LevelObjectData>* all_objects = nullptr;

	bool operator< (const LevelObjectData& rhs) const {
		return level_id < rhs.level_id;
	}

    const LevelObjectData* get_sibling(ObjLevelID obj_id) const {
        const LevelObjectData* ref = nullptr;

        if (obj_id && all_objects) {
            auto it = std::find_if(
                    all_objects->begin(), all_objects->end(),
                    [obj_id](const LevelObjectData& ref) {
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

	std::vector<LevelObjectData> objects;

	unsigned getID() const { return layer_id; }
};

}
