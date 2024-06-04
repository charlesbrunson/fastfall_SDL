#pragma once

#include "fastfall/engine/imgui/ImGuiContent.hpp"

#include "fastfall/resource/asset/SpriteAsset.hpp"
#include "fastfall/resource/asset/TilesetAsset.hpp"
#include "fastfall/resource/asset/LevelAsset.hpp"
#include "fastfall/resource/asset/FontAsset.hpp"
#include "fastfall/resource/asset/ShaderAsset.hpp"
#include "fastfall/resource/asset/SoundAsset.hpp"
#include "fastfall/resource/asset/MusicAsset.hpp"

#include <map>
#include <memory>
#include <typeindex>

namespace ff {


template<typename T>
concept is_asset = (std::derived_from<T, Asset> && !std::same_as<T, Asset>);

template<is_asset T>
struct asset_type {
    using map = std::map<std::string, std::unique_ptr<T>, std::less<>>;

    std::filesystem::path extension;
    std::string_view type_name;
    map assets;
};

class Resources : public ImGuiContent {
private:
	static Resources resource;

	Resources();
    void init_asset_types();

    asset_type<ShaderAsset>  shaders;
    asset_type<SpriteAsset>  sprites;
    asset_type<TilesetAsset> tilesets;
    asset_type<LevelAsset>   levels;
    asset_type<FontAsset>    fonts;
    asset_type<SoundAsset>   sounds;
    asset_type<MusicAsset>   music;

    std::filesystem::path curr_root;

    constexpr auto all_asset_types() {
        return std::tie(
            shaders,
            sprites,
            tilesets,
            levels,
            fonts,
            sounds,
            music
        );
    }

    constexpr void for_each_asset_map(auto&& fn) {
        std::apply([&](auto&&... type) {
            (fn(type.assets), ...);
        }, all_asset_types());
    }

    constexpr void for_each_asset_type(auto&& fn) {
        std::apply([&](auto&&... type) {
            (fn(type), ...);
        }, all_asset_types());
    }

    constexpr void for_each_asset(auto&& fn) {
        for_each_asset_map([&](auto& map) {
            for (auto& it : map) {
                fn(it.second.get());
            }
        });
    }

    template<is_asset T>
    constexpr typename asset_type<T>::map& asset_map_for() {
        return std::get<asset_type<T>&>(all_asset_types()).assets;
    }

    bool loadAssetsFromDirectory(const std::filesystem::path& asset_dir);



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
        else {
            LOG_ERR_("Asset {} not found", filename);
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

    static bool loadAll(std::filesystem::path root);
    static void unloadAll();
	static bool reloadOutOfDateAssets();

    static void ImGui_init() { resource.ImGui_addContent(); }

	void ImGui_getContent(secs deltaTime) override;
};

}