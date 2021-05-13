#pragma once

#include "fastfall/engine/imgui/ImGuiContent.hpp"

#include "fastfall/resource/asset/SpriteAsset.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"
#include "fastfall/resource/asset/LevelAsset.hpp"

#include <map>
#include <memory>

namespace ff {

template<class T>
using AssetMap = std::map<std::string, std::unique_ptr<T>, std::less<>>;

//prevent bad types

template<class Type>
using AssetOnly = std::enable_if<
	std::is_same<Type, SpriteAsset>::value ||
	std::is_same<Type, TilesetAsset>::value
>;


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

	std::map<std::pair<std::string, std::string>, AnimID> anim_lookup_table;
	std::map<AnimID, Animation> animation_table;

	void loadAnimations();

	template<class Type>
	Type* get(AssetMap<Type>& map, const std::string_view filename);

	AssetSource loadMethod;

public:

	static void addLoadedToWatcher();

	static inline AssetSource lastLoadMethod() noexcept { return resource.loadMethod; };

	// function defined via macro
	template<class Type, class = AssetOnly<Type>>
	static Type* get(const std::string_view filename);

	// function defined via macro
	template<class Type, class = AssetOnly<Type>>
	static const Type& add(const std::string& name, std::unique_ptr<Type>&& sprite);

	static void add_animation(const SpriteAsset::ParsedAnim& panim);

	static const Animation* get_animation(AnimID id);
	static AnimID get_animation_id(std::string_view sprite_name, std::string_view anim_name);

	//static const TilesetAsset& add(const std::string& name, std::unique_ptr<TilesetAsset>&& sprite);
	//static const LevelAsset& add(const std::string& name, std::unique_ptr<LevelAsset>&& level);

	// specify all assets
	static bool loadAll(AssetSource loadType, const std::string& filename);
	static void unloadAll();
	static bool buildPackFile(const std::string& packFilename);


	void ImGui_getContent();
};

}