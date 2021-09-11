#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/Vec2.hpp"

#include <map>
#include <vector>
#include <memory>
#include <variant>

namespace ff {

static constexpr unsigned GID_INVALID = 0xFFFFFFFF;
using gid = unsigned int;

// map of the first gid mapped to the tileset name
using TilesetMap = std::map<gid, std::string>;

// represents a tile
struct TileRef {
	gid tile_id = GID_INVALID;
	Vec2u tilePos;
	Vec2u texPos;
	//const std::string* tilesetName = nullptr;
	std::string_view tilesetName;
};

// represents a layer of tile data
struct TileLayerRef {
	bool has_parallax = false;
	Vec2u innerSize;
	Vec2u tileSize;
	Vec2f scrollrate;
	std::vector<TileRef> tiles;
};

struct ObjectLayerRef;

using object_id = unsigned int;
constexpr object_id object_null = 0;

struct ObjectRef {

	object_id id = object_null;
	std::string name;
	size_t type = 0; // hash of type string
	Vec2i position;
	unsigned width = 0u;
	unsigned height = 0u;
	std::vector<std::pair<std::string, std::string>> properties;
	std::vector<Vec2i> points;
};

struct ObjectLayerRef {
	std::vector<ObjectRef> objects;
};

class LayerRef {
public:
	enum class Type {
		Tile,
		Object
	};

	LayerRef(Type Type) :
		type(Type)
	{
		if (type == Type::Tile) {
			layer = TileLayerRef{};
		}
		else {
			layer = ObjectLayerRef{};
		}
	}

	LayerRef(LayerRef&& ref) noexcept
		: layer(std::move(ref.layer))
	{
		id = ref.id;
		type = ref.type;
	}

	unsigned int id = 0;
	Type type;

	constexpr TileLayerRef& asTileLayer() {
		return std::get<TileLayerRef>(layer);
	}
	constexpr ObjectLayerRef& asObjLayer() {
		return std::get<ObjectLayerRef>(layer);
	}
	constexpr const TileLayerRef& asTileLayer() const {
		return std::get<TileLayerRef>(layer);
	}
	constexpr const ObjectLayerRef& asObjLayer() const {
		return std::get<ObjectLayerRef>(layer);
	}
	std::variant<TileLayerRef, ObjectLayerRef> layer;
};

}