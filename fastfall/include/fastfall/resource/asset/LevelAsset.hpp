#pragma once

#include <set>

#include "fastfall/resource/Asset.hpp"
#include "fastfall/util/math.hpp"

#include "fastfall/game/level/LevelLayerContainer.hpp"
#include "fastfall/resource/asset/LevelAssetTypes.hpp"
#include "fastfall/resource/asset/TileLayerData.hpp"
#include "fastfall/render/util/Color.hpp"

namespace ff {

class LevelAsset : public Asset {
public:
	using Layers = LevelLayerContainer<TileLayerData, ObjectLayerData>;

	LevelAsset(const std::filesystem::path& t_asset_path);

	bool loadFromFile() override;

	bool reloadFromFile() override;

	inline Color getBGColor() const { return backgroundColor; };
	inline const Vec2u& getTileDimensions() const { return lvlTileSize; };

	inline const Layers& getLayerRefs() const { return layers; };

	void ImGui_getContent() override;

    std::vector<std::filesystem::path> getDependencies() const override {
        return { asset_path };
    }

protected:
	Color backgroundColor;
	Vec2u lvlTileSize;

	Layers layers;

};

//template<>
//struct flat_type<LevelAsset>
//{
//	using type = flat::resources::LevelAssetF;
//};

}