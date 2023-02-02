#include "fastfall/resource/Resources.hpp"

#include "fastfall/util/log.hpp"

#include "rapidxml.hpp"
using namespace rapidxml;

#include <mutex>
#include <future>

//#include "fastfall/schema/resource-flat.hpp"

#include "fastfall/resource/ResourceWatcher.hpp"
#include "fastfall/resource/ResourceSubscriber.hpp"

#ifndef FF_DATAPATH
#if defined(DEBUG)
#define FF_DATAPATH FF_DATA_DIR
#else
#define FF_DATAPATH ""
#endif
#endif

namespace ff {

Resources Resources::resource;

Resources::Resources() :
	ImGuiContent(ImGuiContentType::SIDEBAR_LEFT, "Resources", "System")
{
    init_asset_types();
}

void Resources::init_asset_types() {

    shaders.type_name  = "Shaders";
    sprites.type_name  = "Sprites";
    tilesets.type_name = "Tilesets";
    levels.type_name   = "Levels";
    fonts.type_name    = "Fonts";
    sounds.type_name   = "Sound Effects";
    music.type_name    = "Music";

    shaders.extension  = ".vert";
    sprites.extension  = ".sax";
    tilesets.extension = ".tsx";
    levels.extension   = ".tmx";
    fonts.extension    = ".ttf";
    sounds.extension   = ".wav";
    music.extension    = ".mp3";
}

bool Resources::loadAll() {
	bool result;
    resource.for_each_asset_type([]<is_asset T>(asset_type<T>& type) {
        type.assets.clear();
    });
    result = resource.loadAssetsFromDirectory( std::string{FF_DATAPATH} + "data/" );
	loadControllerDB();
	return result;
}
void Resources::unloadAll()
{
    resource.for_each_asset_type([]<is_asset T>(asset_type<T>& type) {
        type.assets.clear();
    });
	AnimID::resetCounter();
	Texture::destroyNullTexture();
    AnimDB::reset();
	LOG_INFO("All resources unloaded");
}

bool Resources::loadAssetsFromDirectory(const std::filesystem::path& asset_dir)
{
    for_each_asset_type([&]<is_asset T>(asset_type<T>& type) {
        type.assets.clear();
    });

    namespace fs = std::filesystem;
    for (auto& entry : fs::recursive_directory_iterator(asset_dir)) {
        if (entry.is_regular_file()) {
            auto& path = entry.path();
            bool added = false;
            for_each_asset_type([&]<is_asset T>(asset_type<T>& type) {
                if (!added && type.extension == path.extension()) {
                    added = true;
                    if (type.assets.contains(path.filename().generic_string())) {
                        LOG_WARN("Asset name conflict detected, skipping:", path.generic_string());
                        log::scope sc;
                        Asset* existing = type.assets.at(path.filename().generic_string()).get();
                        LOG_WARN("Existing: {}", existing->get_path().generic_string());
                        LOG_WARN("Skipped:  {}", path.generic_string());
                        return;
                    }
                    auto ptr = std::make_unique<T>(path);
                    type.assets.emplace(ptr->get_name(), std::move(ptr));
                }
            });
        }
    }

	LOG_INFO("Loading assets");
    LOG_INFO("");
    bool r = true;

    for_each_asset_type([&]<is_asset T>(asset_type<T>& type) {
        if (type.assets.empty())
            return;

        LOG_INFO("+-{:-<20}----------------------------------------+", type.type_name);
        {
            log::scope sc;
            for (auto &[name, asset]: type.assets) {
                bool success = asset->loadFromFile();
                r &= success;
                if (success) {
                    LOG_INFO("{:40} ...       complete", asset->get_name());
                    asset->postLoad();
                } else {
                    LOG_ERR_("{:40} ... failed to load", asset->get_name());
                }
            }
        }
        LOG_INFO("+-------------------------------------------------------------+");
        LOG_INFO("");
    });

	return r;
}

void Resources::ImGui_getContent() {
	if (ImGui::CollapsingHeader("Sprites", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (auto& [name, asset] : sprites.assets) {
			if (ImGui::BeginChild(name.c_str(), ImVec2(0, 100), true)) {
				asset->ImGui_getContent();
			}
			ImGui::EndChild();
		}
	}
	if (ImGui::CollapsingHeader("Tilesets", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (auto& [name, asset] : tilesets.assets) {
            if (ImGui::BeginChild(name.c_str(), ImVec2(0, 100), true)) {
                asset->ImGui_getContent();
            }
			ImGui::EndChild();
		}
	}
	if (ImGui::CollapsingHeader("Levels", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (auto& [name, asset] : levels.assets) {
            if (ImGui::BeginChild(name.c_str(), ImVec2(0, 100), true)) {
                asset->ImGui_getContent();
            }
			ImGui::EndChild();
		}
	}
}


void Resources::addLoadedToWatcher() {
    size_t asset_count = 0;
    resource.for_each_asset_map([&](auto& asset_map) {
        for (auto& [key, val] : asset_map) {
            ResourceWatcher::add_watch(
                val.get(),
                val->getDependencies()
            );
            asset_count++;
        }
    });
    LOG_INFO("Watching asset {} files", asset_count);
}


void Resources::loadControllerDB() {
	static bool loadedControllerDB = false;
	if (!loadedControllerDB) {
		std::string path = FF_DATAPATH + std::string("gamecontrollerdb.txt");
		if (SDL_GameControllerAddMappingsFromFile(path.c_str()) >= 0) {
			LOG_INFO("Loaded gamecontrollerdb.txt");
		}
		else {
			LOG_WARN("Unable to load gamecontrollerdb.txt: {}", path);
		}
		loadedControllerDB = true;
	}
}


bool Resources::reloadOutOfDateAssets()
{
	std::vector<const Asset*> assets_changed;

    // reload assets
    resource.for_each_asset([&](Asset* asset) {
        if (asset->isOutOfDate() && asset->isLoaded())
        {
            LOG_INFO("Reloading asset \"{}\"", asset->get_path().generic_string());
            bool reloaded = asset->reloadFromFile();
            asset->setOutOfDate(false);

            log::scope sc;
            if (reloaded) {
                asset->postLoad();
                assets_changed.push_back(asset);
                LOG_INFO("... complete!");
            }
            else {
                LOG_ERR_("... failed to reload asset");
            }
        }
    });

	for (auto asset : assets_changed) {
        auto& all_subs = ResourceSubscriber::get_asset_subscriptions();
		for (auto subscriber : all_subs) {
			if (subscriber->is_subscribed_to_asset(asset)) {
				subscriber->notifyReloadedAsset(asset);
			}
		}
	}

	return assets_changed.size() > 0;
}

}
