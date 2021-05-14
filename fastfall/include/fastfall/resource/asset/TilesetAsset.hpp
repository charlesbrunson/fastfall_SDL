#pragma once

#include "rapidxml.hpp"

#include "fastfall/game/level/Tile.hpp"

#include "fastfall/resource/asset/TextureAsset.hpp"
#include "fastfall/util/math.hpp"

#include "fastfall/schema/resource-flat.hpp"
#include <map>

namespace ff {

class TilesetAsset : public TextureAsset {
public:

	TilesetAsset(const std::string& filename);

	bool loadFromFile(const std::string& relpath);
	bool loadFromFlat(const flat::resources::TilesetAssetF* builder);
	flatbuffers::Offset<flat::resources::TilesetAssetF> writeToFlat(flatbuffers::FlatBufferBuilder& builder) const;

	bool reloadFromFile() override {
		//TODO
		return false;
	};

	Tile getTile(Vec2u texPos) const;
	inline const Vec2u& getTileSize() { return texTileSize; };

	void ImGui_getContent();

	inline const std::string_view getTilesetRef(unsigned ndx) const { return tilesetRef.at(ndx); };

protected:
	void parseTileProperties(rapidxml::xml_node<>* propsNode, Tile& t);

	// references to other tilesets
	std::vector<std::string> tilesetRef;

	std::map<Vec2u, Tile> tileData;
	Vec2u texTileSize;
};

template<>
struct flat_type<TilesetAsset>
{
	using type = flat::resources::TilesetAssetF;
};

}
