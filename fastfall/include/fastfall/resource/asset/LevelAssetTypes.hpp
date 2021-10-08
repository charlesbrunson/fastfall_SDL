#pragma once

#include "fastfall/util/math.hpp"
#include "fastfall/util/Vec2.hpp"
#include "fastfall/util/log.hpp"

#include <map>
#include <vector>
#include <memory>
#include <variant>

#include "fastfall/resource/asset/TileLayerData.hpp"

namespace ff {



using object_id = unsigned int;
constexpr object_id object_null = 0;

struct ObjectData {
	object_id id = object_null;
	std::string name;
	size_t typehash = 0; // hash of type string
	Vec2i position;
	unsigned width = 0u;
	unsigned height = 0u;
	std::vector<std::pair<std::string, std::string>> properties;
	std::vector<Vec2i> points;
};

struct ObjectLayerData {
	std::vector<ObjectData> objects;
};

class LayerData {
public:
	enum class Type {
		Tile,
		Object
	};

	LayerData(Type Type) :
		type(Type)
	{
		if (type == Type::Tile) {
			layer = TileLayerData{};
		}
		else {
			layer = ObjectLayerData{};
		}
	}

	LayerData(TileLayerData&& tile) :
		type(Type::Tile)
	{
		layer = std::move(tile);
	}
	LayerData(ObjectLayerData&& obj) :
		type(Type::Object)
	{
		layer = std::move(obj);
	}

	LayerData(LayerData&& data) noexcept
		: layer(std::move(data.layer))
	{
		id = data.id;
		type = data.type;
	}

	unsigned int id = 0;
	Type type;

	constexpr TileLayerData& asTileLayer() {
		return std::get<TileLayerData>(layer);
	}
	constexpr ObjectLayerData& asObjLayer() {
		return std::get<ObjectLayerData>(layer);
	}
	constexpr const TileLayerData& asTileLayer() const {
		return std::get<TileLayerData>(layer);
	}
	constexpr const ObjectLayerData& asObjLayer() const {
		return std::get<ObjectLayerData>(layer);
	}
	std::variant<TileLayerData, ObjectLayerData> layer;
};

}
