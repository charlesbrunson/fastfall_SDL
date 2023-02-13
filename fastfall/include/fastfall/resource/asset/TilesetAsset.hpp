#pragma once

#include "rapidxml.hpp"


#include "fastfall/game/tile/Tile.hpp"
#include "fastfall/resource/asset/TextureAsset.hpp"

#include "fastfall/util/math.hpp"
#include "fastfall/util/grid_vector.hpp"

#include "fastfall/engine/time/time.hpp"

//#include "fastfall/schema/resource-flat.hpp"
#include <map>
#include <optional>

namespace ff {


class TilesetAsset : public TextureAsset {
protected:
	enum TileHasProp {
		HasTile			= 1 << 0,
		HasLogic		= 1 << 1,
		HasLogicArgs	= 1 << 2,
		HasMaterial		= 1 << 3,
		HasConstraint	= 1 << 4,
        HasFrameCount   = 1 << 5
	};

	struct TileData {
		Tile     tile;
        uint8_t  has_prop_bits = 0;
        uint8_t  frameCount = 1;    // how many frames this tile has
        uint8_t  frameDelay = 60;   // in 1/60 second increments
		unsigned tileLogicNdx;		// ndx of tileLogic
		unsigned tileLogicParamNdx; // ndx of tileLogic's parameter list
		unsigned tileMatNdx;		// ndx of tileMat
		unsigned tileConstraint;    // ndx of autotile constraints for this tile
	};

	struct TilesetLogic {
		std::string logicType;
		std::vector<std::string> logicArg;
	};

	//std::unique_ptr<TileData[]> tiles;
	Vec2u texTileSize;
	grid_vector<TileData> tiles;

	std::vector<std::string>	tilesetRef;	// name of tileset reference
	std::vector<TilesetLogic>	tileLogic;	// name and params of tile logic
	std::vector<std::string>	tileMat;	// name of material
	std::vector<TileConstraint> constraints;
    std::vector<TileConstraint> frameCount;

	const static std::map<std::string, void(*)(TilesetAsset&, TileData&, char*)> tileProperties;

	unsigned addLogicType(std::string type);
	unsigned addLogicArgs(unsigned logicType, std::string args);
	unsigned addMaterial(std::string mat);
	unsigned addTileConstraint(TileID tile_id, TileConstraint constraint);

public:
    struct TileLogicData {
        std::string_view logic_type;
        std::string_view logic_args;
    };

	TilesetAsset(const std::filesystem::path& t_asset_path);

	bool loadFromFile() override;
	bool reloadFromFile() override;

	std::optional<Tile> getTile(TileID tile_id) const;
	inline const Vec2u& getTileSize() const { return texTileSize; };

	void ImGui_getContent() override;

	inline const std::string_view getTilesetRef(unsigned ndx) const { return tilesetRef.at(ndx); };

	unsigned getTilesetRefIndex(std::string_view tileset_name);

	TileLogicData       getTileLogic (TileID tile_id) const;
	const TileMaterial& getMaterial  (TileID tile_id) const;
    unsigned            getFrameCount(TileID tile_id) const;
    secs                getFrameDelay(TileID tile_id) const;

	const std::vector<TileConstraint>& getConstraints() const { return constraints; };
	std::optional<TileID> getAutoTileForShape(TileShape shape) const;

    std::vector<std::filesystem::path> getDependencies() const override {
        return { get_path(), get_texture_path() };
    }

protected:
	void loadFromFile_TileProperties(rapidxml::xml_node<>* propsNode, TileData& t);
	void loadFromFile_Header(rapidxml::xml_node<>* tileset_node);
	void loadFromFile_Tile(rapidxml::xml_node<>* tile_node);

	mutable std::vector<std::pair<TileShape, TileID>> auto_shape_cache;

};

}
