#pragma once


#include "fastfall/resource/Asset.hpp"
#include "fastfall/util/math.hpp"
//#include "phys/CollisionMap.hpp"

#include "fastfall/resource/asset/LevelAssetTypes.hpp"
#include "fastfall/render/Color.hpp"

//#include <SFML/Graphics.hpp>
#include "fastfall/schema/resource-flat.hpp"

namespace ff {

// max size of a level in either direction
// really arbitrary right now
constexpr int LEVEL_DIMENSION_MAX = 256;


class LevelAsset : public Asset {
public:
	LevelAsset(const std::string& filename);

	bool loadFromFile(const std::string& relpath) override;
	bool loadFromFlat(const flat::resources::LevelAssetF* builder);
	flatbuffers::Offset<flat::resources::LevelAssetF> writeToFlat(flatbuffers::FlatBufferBuilder& builder) const;

	bool reloadFromFile() override {
		//TODO
		return false;
	}

	inline Color getBGColor() const { return backgroundColor; };
	inline const Vec2u& getTileDimensions() const { return lvlTileSize; };

	inline const TilesetMap* getTilesetDependencies() const { return &tilesetDeps; };
	inline const std::vector<LayerRef>* getLayerRefs() const { return &layers; };

	inline unsigned getBorder() const { return borderCardinalBits; };

	void ImGui_getContent() override;

protected:
	//void generateCollisionForLayer(LayerRef& ref);

	Color backgroundColor;
	Vec2u lvlTileSize;

	// first gid, std::string
	TilesetMap tilesetDeps;
	std::vector<LayerRef> layers;

	unsigned borderCardinalBits = 0u;

};

template<>
struct flat_type<LevelAsset>
{
	using type = flat::resources::LevelAssetF;
};

}