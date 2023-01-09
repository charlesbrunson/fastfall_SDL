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
private:
	static Resources resource;

	Resources();

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

    template<is_asset T>
    constexpr AssetMap<T>& asset_map_for() {
        return std::get<AssetMap<T>&>(all_asset_maps());
    }

public:
	static void loadControllerDB();
	static void addLoadedToWatcher();

	template<is_asset Type>
	static Type* get(std::string_view filename) {
        auto& map = resource.asset_map_for<Type>();
        auto it = map.find(filename);
        if (it != map.end()) {
            return it->second.get();
        }
        return nullptr;
    }

	template<is_asset Type>
	static const Type& add(std::string_view filename, std::unique_ptr<Type>&& asset) {
        static std::mutex mut;
        const std::lock_guard<std::mutex> lock(mut);
        auto r = resource.asset_map_for<Type>().emplace(filename, std::move(asset));
        return *r.first->second.get();
    }

    static bool loadAll(std::string_view filename);
    static void unloadAll();
    static bool compileShaders();
	static bool reloadOutOfDateAssets();

	void ImGui_getContent();
};

}