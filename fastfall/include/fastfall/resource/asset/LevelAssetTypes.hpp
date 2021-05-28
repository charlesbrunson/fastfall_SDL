#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/Vec2.hpp"

#include <map>
#include <vector>
#include <memory>

namespace ff {

static constexpr unsigned GID_INVALID = 0xFFFFFFFF;
using gid = unsigned int;

// map of the first gid mapped to the tileset name
using TilesetMap = std::map<gid, std::string>;

// represents a tile
struct TileRef {
	gid tile_id = GID_INVALID;
	Vec2u texPos;
	const std::string* tilesetName = nullptr;
};

using object_id = unsigned int;
constexpr object_id object_null = 0;

// represents a layer of tile data
struct TileLayerRef {

	bool isParallax = false;
	Vec2u internalSize;
	Vec2u tileSize;
	std::map<Vec2u, TileRef> tiles;

	//bool isActive = false;
};

struct ObjectLayerRef;

class ObjectRef {
public:
	ObjectRef(ObjectLayerRef* layer) : layerRef(layer) {

	};
	ObjectRef() = delete;

	object_id id;
	std::string name;
	size_t type; // hash of type string
	Vec2i position;
	unsigned width = 0u;
	unsigned height = 0u;
	std::vector<std::pair<std::string, std::string>> properties;
	std::vector<Vec2i> points;

	inline const ObjectLayerRef* getLayer() const { return layerRef; };

protected:
	ObjectLayerRef* layerRef = nullptr;
};

struct ObjectLayerRef {
	std::map<object_id, ObjectRef> objects;
};

enum class LayerType {
	TILELAYER,
	OBJECTLAYER
};

class LayerRef {
public:
	LayerRef(LayerType Type) :
		type(Type)
	{
		if (type == LayerType::TILELAYER) {
			tileLayer = std::make_unique<TileLayerRef>();
		}
		else {
			objLayer = std::make_unique<ObjectLayerRef>();
		}
	}
	~LayerRef() {

	}

	LayerRef(LayerRef&& ref) noexcept
	{
		id = ref.id;
		type = ref.type;
		tileLayer.swap(ref.tileLayer);
		objLayer.swap(ref.objLayer);
	}

	unsigned int id = 0;

	std::unique_ptr<TileLayerRef> tileLayer = nullptr;
	std::unique_ptr<ObjectLayerRef> objLayer = nullptr;
	LayerType type;
};

}