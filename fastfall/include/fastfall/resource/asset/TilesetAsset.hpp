#pragma once

#include "rapidxml.hpp"

#include "fastfall/game/level/Tile.hpp"
#include "fastfall/resource/asset/TextureAsset.hpp"

#include "fastfall/util/math.hpp"

#include "fastfall/schema/resource-flat.hpp"
#include <map>
#include <optional>

namespace ff {


class TilesetAsset : public TextureAsset {
protected:
	enum TileHasProp {
		HasTile = 1 << 0,
		HasLogic = 1 << 1,
		HasLogicArgs = 1 << 2,
		HasMaterial = 1 << 3
	};

	struct TileData {
		uint8_t has_prop_bits = 0;
		Tile tile;
		unsigned tileLogicNdx;		// ndx of tileLogic
		unsigned tileLogicParamNdx; // ndx of tileLogic's parameter list
		unsigned tileMatNdx;		// ndx of tileMat
	};

	struct TilesetLogic {
		std::string logicType;
		std::vector<std::string> logicArg;
	};

	Vec2u texTileSize;
	std::unique_ptr<TileData[]> tiles;
	std::vector<std::string> tilesetRef;	// name of tileset reference
	std::vector<TilesetLogic> tileLogic;	// name and params of tile logic
	std::vector<std::string> tileMat;		// name of material

	const static std::map<std::string, void(*)(TilesetAsset&, TileData&, char*)> tileProperties;

	unsigned addTilesetRef(std::string ref);
	unsigned addLogicType(std::string type);
	unsigned addLogicArgs(unsigned logicType, std::string args);
	unsigned addMaterial(std::string mat);

public:

	TilesetAsset(const std::string& filename);

	bool loadFromFile(const std::string& relpath) override;
	bool loadFromFlat(const flat::resources::TilesetAssetF* builder);
	flatbuffers::Offset<flat::resources::TilesetAssetF> writeToFlat(flatbuffers::FlatBufferBuilder& builder) const;

	bool reloadFromFile() override;

	Tile getTile(Vec2u texPos) const;
	inline const Vec2u& getTileSize() const { return texTileSize; };

	void ImGui_getContent() override;

	inline const std::string_view getTilesetRef(unsigned ndx) const { return tilesetRef.at(ndx); };

	int getTilesetRefIndex(std::string_view tileset_name);

	struct TileLogicData {
		std::string_view logic_type;
		std::string_view logic_args;
	};

	TileLogicData getTileLogic(Vec2u position) const;
	const TileMaterial& getMaterial(Vec2u position) const;

protected:
	
	void loadFromFile_TileProperties(rapidxml::xml_node<>* propsNode, TileData& t);
	void loadFromFile_Header(rapidxml::xml_node<>* tileset_node, const std::string_view& relpath);
	void loadFromFile_Tile(rapidxml::xml_node<>* tile_node);

	constexpr size_t get_ndx(Vec2u pos) const {
		assert(pos.x < texTileSize.x && pos.y < texTileSize.y);
		return (size_t)pos.x + ((size_t)pos.y * texTileSize.x);
	}
};

template<>
struct flat_type<TilesetAsset>
{
	using type = flat::resources::TilesetAssetF;
};

}
