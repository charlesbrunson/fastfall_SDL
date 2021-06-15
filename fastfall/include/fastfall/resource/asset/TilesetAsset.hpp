#pragma once

#include "rapidxml.hpp"

#include "fastfall/resource/asset/Tile.hpp"
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

protected:

	void setTileLogic(Vec2u position, const TileLogicData& logic) {
		logicData.insert(std::make_pair(position, logic));
	}

	void parseTileProperties(rapidxml::xml_node<>* propsNode, Tile& t);

	// references to other tilesets
	std::vector<std::string> tilesetRef;

	std::map<Vec2u, Tile> tileData;
	std::map<Vec2u, TileLogicData> logicData;
	Vec2u texTileSize;
};

template<>
struct flat_type<TilesetAsset>
{
	using type = flat::resources::TilesetAssetF;
};

}
