#include "fastfall/resource/Resources.hpp"

#include "fastfall/util/log.hpp"

#include "rapidxml.hpp"
using namespace rapidxml;

#include <fstream>
#include <assert.h>
#include <mutex>
#include <future>

//#include <ranges>

#include "fastfall/schema/resource-flat.hpp"

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
}

bool loadFromIndex(std::string_view indexFile);

bool Resources::loadAll(std::string_view filename) {
	bool result;
	resource.ImGui_addContent();
    result = loadFromIndex(filename);
	loadControllerDB();
	return result;
}
void Resources::unloadAll()
{
	resource.fonts.clear();
	AnimID::resetCounter();
	resource.tilesets.clear();
	resource.sprites.clear();
	Texture::destroyNullTexture();
	resource.levels.clear();
    resource.shaders.clear();
    AnimDB::reset();
	LOG_INFO("All resources unloaded");
}

////////////////////////////////////////////////////////////

// indexfile parsing
std::vector<std::string> parseDataNodes(xml_node<>* first_node, const char* type);

struct AssetInfo {
    std::string path;
    std::vector<std::string> asset_names;
};

template<is_asset Type>
bool loadAssets(const AssetInfo& info) {

	for (const auto& asset : info.asset_names)
	{
		std::unique_ptr<Type> ptr = std::make_unique<Type>(asset);
		std::string small_path = info.path.substr(info.path.rfind("data/") + 5);

		log::scope scope;
		if (ptr->loadFromFile(info.path)) {
			Resources::add(asset, std::move(ptr));
			LOG_INFO("{}{} ... complete", small_path, asset);
		}
		else {
			LOG_ERR_("{0}{1} ... failed to load: {2}, {1}", small_path, asset, typeid(*ptr.get()).name());
			return false;
		}
	}
	return true;
}


bool loadFromIndex(std::string_view indexFile)
{
	bool r = true;
    AssetInfo shader_info;
    AssetInfo sprite_info;
    AssetInfo tileset_info;
    AssetInfo level_info;
    AssetInfo font_info;

    std::map<std::string, std::pair<std::string, AssetInfo*>> all_info{
        { "shaders",  { "shader",  &shader_info  } },
        { "sprites",  { "sprite",  &sprite_info  } },
        { "tilesets", { "tileset", &tileset_info } },
        { "levels",   { "level",   &level_info   } },
        { "fonts",    { "font",    &font_info    } },
    };

    std::string dataPath = std::string(FF_DATAPATH) + "data/";

	// try to open the index file
	std::ifstream ndxStream(dataPath + std::string{indexFile}, std::ios::binary | std::ios::ate);
	if (ndxStream.is_open()) {
		char* datatypePath;

		//parse index file
		std::streamsize len = ndxStream.tellg();
		ndxStream.seekg(0, std::ios::beg);

		auto indexContent = std::make_unique<char[]>(len + 1);

		ndxStream.read(&indexContent[0], len);
		indexContent[len] = '\0';
		ndxStream.close();

		auto doc = std::make_unique<xml_document<>>();
		try {
			doc->parse<0>(&indexContent[0]);
			xml_node<>* index = doc->first_node("index");
			if (index) {
				xml_node<>* datatype = index->first_node();
				while (datatype) {
					char* name = datatype->name();
					datatypePath = datatype->first_attribute("path")->value();

                    if (auto it = all_info.find(name);
                        it != all_info.end())
                    {
                        auto& [xml_type, info] = it->second;
                        info->path = dataPath + datatypePath;
                        info->asset_names = parseDataNodes(datatype->first_node(), xml_type.c_str());
                    }
                    else {
                        LOG_WARN("error parsing index file: unknown asset type {}", name);
                    }
					datatype = datatype->next_sibling();
				}
			}
			else {
				throw parse_error("no index", nullptr);
			}
		}
		catch (parse_error err) {
			LOG_ERR_(err.what());
			r = false;
		}
	}
	else {
		r = false;
	}

	LOG_INFO("Loading assets");
    r &= loadAssets<ShaderAsset >(shader_info);
	r &= loadAssets<FontAsset   >(font_info);
	r &= loadAssets<SpriteAsset >(sprite_info);
	r &= loadAssets<TilesetAsset>(tileset_info);
	r &= loadAssets<LevelAsset  >(level_info);
	return r;
}

std::vector<std::string> parseDataNodes(xml_node<>* first_node, const char* type) {

	xml_node<>* node = first_node;
	std::vector<std::string> names;

	while (node) {
		if (auto name_attr = node->first_attribute("name");
			strcmp(node->name(), type) == 0	&& name_attr)
		{
			names.push_back(name_attr->value());
		}
		else {
			throw parse_error("incorrect type", node->name());
		}
		node = node->next_sibling();
	}
	return std::move(names);
}

void Resources::ImGui_getContent() {
	if (ImGui::CollapsingHeader("Sprites", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (auto& asset : sprites) {
			if (ImGui::BeginChild(asset.second.get()->getAssetName().c_str(), ImVec2(0, 100), true)) {
				asset.second.get()->ImGui_getContent();
			}
			ImGui::EndChild();
		}
	}
	if (ImGui::CollapsingHeader("Tilesets", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (auto& asset : tilesets) {
			if (ImGui::BeginChild(asset.second.get()->getAssetName().c_str(), ImVec2(0, 100), true)) {
				asset.second.get()->ImGui_getContent();
			}
			ImGui::EndChild();
		}
	}
	if (ImGui::CollapsingHeader("Levels", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (auto& asset : levels) {
			if (ImGui::BeginChild(asset.second.get()->getAssetName().c_str(), ImVec2(0, 100), true)) {
				asset.second.get()->ImGui_getContent();
			}
			ImGui::EndChild();
		}
	}
}


void Resources::addLoadedToWatcher() {
    resource.for_each_asset_map([](auto& asset_map) {
        for (auto& [key, val] : asset_map) {
            ResourceWatcher::add_watch(
                val.get(),
                val->getDependencies()
            );
        }
    });
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
            LOG_INFO("Reloading asset \"{}\"", asset->getAssetName());
            bool reloaded = asset->reloadFromFile();
            asset->setOutOfDate(false);

            log::scope sc;
            if (reloaded) {
                assets_changed.push_back(asset);
                LOG_INFO("... complete!");
            }
            else {
                LOG_ERR_("... failed to reload asset");
            }
        }
    });

	for (auto asset : assets_changed) {
        auto& all_subs = ResourceSubscriber::getAll();
		for (auto subscriber : all_subs) {
			if (subscriber->is_subscribed(asset)) {
				subscriber->notifyReloadedAsset(asset);
			}
		}
	}

	return assets_changed.size() > 0;
}

bool Resources::compileShaders() {
    bool allgood = true;
    LOG_INFO("Compiling shaders");
    {
        log::scope sc;
        for (auto &[key, val]: resource.shaders) {
            if (!val->compileShaderFromFile()) {
                LOG_ERR_("{} ... failed to compile.", val->getAssetName());
                allgood = false;
            }
            else {
                LOG_INFO("{} ... compiled.", val->getAssetName());
            }
        }
    }
    return allgood;
}

}
