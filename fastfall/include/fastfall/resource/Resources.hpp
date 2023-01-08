#pragma once

#include "fastfall/engine/imgui/ImGuiContent.hpp"

#include "fastfall/resource/asset/SpriteAsset.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"
#include "fastfall/resource/asset/LevelAsset.hpp"
#include "fastfall/resource/asset/FontAsset.hpp"
#include "fastfall/resource/asset/ShaderAsset.hpp"

#include <map>
#include <memory>

namespace ff {

template<typename T>
concept is_asset = (std::derived_from<T, Asset> && !std::same_as<T, Asset>);

template<is_asset T>
using AssetMap = std::map<std::string, std::unique_ptr<T>, std::less<>>;

class Resources : public ImGuiContent {
public:
	enum class AssetSource {
		INDEX_FILE,
		PACK_FILE
	};

private:
	static Resources resource;

	Resources();
	~Resources();

    AssetMap<ShaderAsset>   shaders;
	AssetMap<SpriteAsset>   sprites;
	AssetMap<TilesetAsset>  tilesets;
	AssetMap<LevelAsset>    levels;
	AssetMap<FontAsset>     fonts;

    constexpr auto all_asset_maps() {
        return std::tie(
            shaders,
            sprites,
            tilesets,
            levels,
            fonts
        );
    }

    constexpr void for_each_asset_map(auto&& fn) {
        std::apply([&](auto&&... map) {
            (fn(map), ...);
        }, all_asset_maps());
    }

    constexpr void for_each_asset(auto&& fn) {
        for_each_asset_map([&](auto& map) {
            for (auto& it : map) {
                fn(it.second.get());
            }
        });
    }

	std::map<std::pair<std::string, std::string>, AnimID> anim_lookup_table;
	std::map<AnimID, Animation> animation_table;

	void loadAnimations();

	template<is_asset Type>
	Type* get(AssetMap<Type>& map, const std::string_view filename);

	AssetSource loadMethod;

public:

	static void loadControllerDB();

	static void addLoadedToWatcher();

	static inline AssetSource lastLoadMethod() noexcept { return resource.loadMethod; };

	// function defined via macro
	template<is_asset Type>
	static Type* get(const std::string_view filename);

	// function defined via macro
	template<is_asset Type>
	static const Type& add(const std::string& name, std::unique_ptr<Type>&& sprite);

	static AnimID add_animation(const SpriteAsset::ParsedAnim& panim);

	static const Animation* get_animation(AnimID id);
	static AnimID get_animation_id(std::string_view sprite_name, std::string_view anim_name);

	// specify all assets
	static bool loadAll(AssetSource loadType, const std::string& filename);
	static void unloadAll();
	static bool buildPackFile(const std::string& packFilename);

    static bool compileShaders();

	// attempt to reload out of date assets, returns true if needed
	static bool reloadOutOfDateAssets();

	void ImGui_getContent();
};

}