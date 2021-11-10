#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/Vec2.hpp"
#include "fastfall/util/log.hpp"

#include <map>
#include <vector>
#include <memory>
#include <variant>

namespace ff {

static constexpr unsigned GID_INVALID = UINT32_MAX;
using gid = uint32_t;

// map of the first gid mapped to the tileset name
using TilesetMap = std::map<gid, std::string>;

using object_id = unsigned int;
constexpr object_id object_null = 0;

struct ObjectData {
	std::string name;
	size_t typehash = 0; // hash of type string
	Vec2i position;
	unsigned width = 0u;
	unsigned height = 0u;
	std::vector<std::pair<std::string, std::string>> properties;
	std::vector<Vec2i> points;
};

struct ObjectDataRef {
	object_id id;
	ObjectData data;
};

struct ObjectLayerData {
	unsigned layer_id = 0;
	std::vector<ObjectDataRef> objects;

	unsigned getID() const { return layer_id; }
};

}
