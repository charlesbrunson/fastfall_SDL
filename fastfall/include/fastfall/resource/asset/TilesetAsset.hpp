#pragma once

#include "rapidxml.hpp"

#include "fastfall/game/level/Tile.hpp"
#include "fastfall/resource/asset/TextureAsset.hpp"

#include "fastfall/util/math.hpp"

#include "fastfall/schema/resource-flat.hpp"
#include <map>
#include <optional>

namespace ff {

struct TileLogicData {
	std::string logicType;
	std::string logicArg;
};

class TilesetAsset : public TextureAsset {
public:

	TilesetAsset(const std::string& filename);

	bool loadFromFile(const std::string& relpath) override;
	bool loadFromFlat(const flat::resources::TilesetAssetF* builder);
	flatbuffers::Offset<flat::resources::TilesetAssetF> writeToFlat(flatbuffers::FlatBufferBuilder& builder) const;

	bool reloadFromFile() override {
		//TODO
		return false;
	};

	Tile getTile(Vec2u texPos) const;
	inline const Vec2u& getTileSize() { return texTileSize; };

	void ImGui_getContent() override;

	inline const std::string_view getTilesetRef(unsigned ndx) const { return tilesetRef.at(ndx); };

	int getTilesetRefIndex(std::string_view tileset_name);

	std::optional<TileLogicData> getTileLogic(Vec2u position) const {
		if (auto it = logicData.find(position); it != logicData.end()) {
			return it->second;
		}
		return std::nullopt;
	}

	const TileMaterial& getMaterial(Vec2u position) const {
		if (auto it = matData.find(position); it != matData.end()) {
			return Tile::getMaterial(it->second);
		}
		return Tile::standardMat;
	}

protected:

	void setTileLogic(Vec2u position, const TileLogicData& logic) {
		logicData.insert(std::make_pair(position, logic));
	}
	void setTileMaterial(Vec2u position, const std::string& material) {
		matData.insert(std::make_pair(position, material));
	}

	void parseTileProperties(rapidxml::xml_node<>* propsNode, Tile& t);

	// references to other tilesets
	std::vector<std::string> tilesetRef;

	std::map<Vec2u, Tile> tileData;
	std::map<Vec2u, TileLogicData> logicData;
	std::map<Vec2u, std::string> matData;
	Vec2u texTileSize;
};

template<>
struct flat_type<TilesetAsset>
{
	using type = flat::resources::TilesetAssetF;
};

}
