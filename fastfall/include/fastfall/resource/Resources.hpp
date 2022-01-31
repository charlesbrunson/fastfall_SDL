#pragma once

#include "fastfall/engine/imgui/ImGuiContent.hpp"

#include "fastfall/resource/asset/SpriteAsset.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"
#include "fastfall/resource/asset/LevelAsset.hpp"
#include "fastfall/resource/asset/FontAsset.hpp"

#include <map>
#include <memory>

namespace ff {

template<class T>
using AssetMap = std::map<std::string, std::unique_ptr<T>, std::less<>>;

//prevent bad types

/*
template<class Type>
using AssetOnly = std::enable_if<
	std::is_same<Type, SpriteAsset>::value ||
	std::is_same<Type, TilesetAsset>::value
>;
*/

template<typename T>
concept is_asset = std::is_same_v<T, SpriteAsset>
				|| std::is_same_v<T, TilesetAsset>
				|| std::is_same_v<T, LevelAsset>
				|| std::is_same_v<T, FontAsset>;



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

	AssetMap<SpriteAsset> sprites;
	AssetMap<TilesetAsset> tilesets;
	AssetMap<LevelAsset> levels;
	AssetMap<FontAsset> fonts;

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

	// attempt to reload out of date assets, returns true if needed
	static bool reloadOutOfDateAssets();

	void ImGui_getContent();
};

}