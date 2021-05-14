#include "fastfall/resource/Resources.hpp"

#include "fastfall/util/log.hpp"

#include "rapidxml.hpp"
using namespace rapidxml;

#include <fstream>
#include <assert.h>
#include <mutex>
#include <future>

#include <ranges>

#include "fastfall/schema/resource-flat.hpp"

#include "fastfall/resource/ResourceWatcher.hpp"

#if defined(DEBUG)
#define DEBUGPATH "../../../"
#else
#define DEBUGPATH ""
#endif

// internal stuff
////////////////////////////////////////////////////////////

namespace ff {

Resources Resources::resource;


Resources::Resources() :
	ImGuiContent(ImGuiContentType::SIDEBAR_LEFT, "Resources", "System")
{

}
Resources::~Resources() {
	//unloadAll();
}

template<class Type>
Type* Resources::get(AssetMap<Type>& map, const std::string_view filename) {
	auto r = map.find(filename);

	if (r != map.end()) {
		return r->second.get();
	}
	return nullptr;
}

////////////////////////////////////////////////////////////

// resource adder
#define ADDRESOURCE(Type, Map)															\
template<>																				\
const Type& Resources::add(const std::string& name, std::unique_ptr<Type>&& asset) {	\
	static std::mutex mut;																\
	const std::lock_guard<std::mutex> lock(mut);										\
	auto r = Map.insert(std::make_pair(name, std::move(asset)));						\
	return *r.first->second.get();														\
}

ADDRESOURCE(SpriteAsset, resource.sprites)
ADDRESOURCE(TilesetAsset, resource.tilesets)
ADDRESOURCE(LevelAsset, resource.levels)

#undef ADDRESOURCE

	////////////////////////////////////////////////////////////

	// resource getters
#define GETRESOURCE(Type, Map)									\
template<>														\
Type* Resources::get<Type>(const std::string_view filename) {	\
	return resource.get(Map, filename);							\
}

GETRESOURCE(SpriteAsset, resource.sprites)
GETRESOURCE(TilesetAsset, resource.tilesets)
GETRESOURCE(LevelAsset, resource.levels)

#undef GETRESOURCE

	////////////////////////////////////////////////////////////

	const Animation* Resources::get_animation(AnimID id) {

	auto iter = resource.animation_table.find(id);

	if (iter != resource.animation_table.end()) {
		return &iter->second;
	}
	return nullptr;
}

AnimID Resources::get_animation_id(std::string_view sprite_name, std::string_view anim_name) {

	auto iter = resource.anim_lookup_table.find(std::pair<std::string, std::string>(sprite_name, anim_name));

	if (iter != resource.anim_lookup_table.end()) {
		return iter->second;
	}
	return AnimID::NONE;
}


void Resources::add_animation(const SpriteAsset::ParsedAnim& panim) {

	auto doChain = [&panim](Animation& anim) {

		if (anim.chain.has_chain) {

			AnimID chain_id = get_animation_id(panim.chain_spr_name, panim.chain_anim_name);

			if (chain_id == AnimID::NONE) {
				// operate under assumption that there will be an anim later on to fill this
				anim.chain.anim_id = AnimID::reserve_id();
				auto chain_key = std::pair<std::string, std::string>(panim.chain_spr_name, panim.chain_anim_name);
				resource.anim_lookup_table.insert(std::make_pair(chain_key, anim.chain.anim_id));
			}
			else {
				anim.chain.anim_id = chain_id;
			}

			anim.chain.start_frame = panim.chain_frame;
		}
	};

	log::scope scope;

	AnimID existing_id = get_animation_id(panim.owner->getAssetName(), panim.name);
	if (existing_id == AnimID::NONE) {
		AnimID nID = AnimID::reserve_id();
		auto key = std::pair<std::string, std::string>(panim.owner->getAssetName(), panim.name);
		resource.anim_lookup_table.insert(std::make_pair(key, nID));


		Animation anim{ panim.owner, nID };
		anim.anim_name = panim.name;
		anim.area = panim.area;
		anim.origin = panim.origin;
		anim.loop = panim.loop;

		anim.framerateMS = panim.framerateMS;
		anim.chain.has_chain = panim.has_chain;
		doChain(anim);

		//resource.animation_table.insert(std::make_pair( anim.anim_id, anim ));
		resource.animation_table[anim.anim_id] = std::move(anim);

	}
	else {
		Animation anim{ panim.owner, existing_id };
		anim.anim_name = panim.name;
		anim.area = panim.area;
		anim.origin = panim.origin;
		anim.loop = panim.loop;

		anim.framerateMS = panim.framerateMS;
		anim.chain.has_chain = panim.has_chain;
		doChain(anim);

		resource.animation_table[existing_id] = std::move(anim);
	}
	LOG_INFO("loaded anim: {} - {}", panim.owner->getAssetName(), panim.name);
}

////////////////////////////////////////////////////////////

bool loadFromPack(const std::string& packFile);
bool loadFromIndex(const std::string& indexFile);

bool Resources::loadAll(AssetSource loadType, const std::string& filename) {
	bool result;

	resource.loadMethod = loadType;

	resource.ImGui_addContent();

	switch (loadType) {
	case AssetSource::INDEX_FILE:
		LOG_INFO("Parsing assets from index file: {}", filename);
		result = loadFromIndex(filename);
		break;
	case AssetSource::PACK_FILE:
		LOG_INFO("Parsing assets from pack file: {}", filename);
		result = loadFromPack(filename);
		break;
	default:
		result = false;
	}

	return result;
}
void Resources::unloadAll() {

	resource.anim_lookup_table.clear();
	resource.animation_table.clear();
	AnimID::resetCounter();
	resource.tilesets.clear();
	resource.sprites.clear();
	resource.levels.clear();
	LOG_INFO("All resources unloaded");
}

////////////////////////////////////////////////////////////

template<typename  AssetType, typename  FlatType = typename flat_type<AssetType>::type>
std::string LoadPackedAsset(const FlatType* flat) {
	std::string filename = flat->name()->c_str();

	auto ptr = std::make_unique<AssetType>(filename);
	bool success = ptr->loadFromFlat(flat);
	if (success) {
		Resources::add(ptr->getAssetName(), std::move(ptr));
		return filename;
	}
	else {
		throw std::runtime_error("failed to unpack " + ptr->getAssetName());
	}
};


bool loadFromPack(const std::string& packFile) {

	std::string path = DEBUGPATH + packFile;

	// load pack file into memory
	bool good = true;
	std::vector<int8_t> packData = readFile(path.c_str());

	LOG_INFO("Deserializing assets");


	//sf::Clock timer;
	auto res = flat::resources::GetResourcesF(packData.data());

	for (auto it = res->sprites()->begin(); it != res->sprites()->end(); it++) {
		log::scope scope;
		try {
			std::string filename = LoadPackedAsset<SpriteAsset>(*it);
			LOG_INFO("{} ... complete", filename);
		}
		catch (std::exception err) {
			LOG_ERR_("failure to load asset: {}", err.what());
			good = false;
		}
	}

	for (auto it = res->tilesets()->begin(); it != res->tilesets()->end(); it++) {
		log::scope scope;
		try {
			std::string filename = LoadPackedAsset<TilesetAsset>(*it);
			LOG_INFO("{} ... complete", filename);
		}
		catch (std::exception err) {
			LOG_ERR_("failure to load asset: {}", err.what());
			good = false;
		}
	}
	for (auto it = res->levels()->begin(); it != res->levels()->end(); it++) {
		log::scope scope;
		try {
			std::string filename = LoadPackedAsset<LevelAsset>(*it);
			LOG_INFO("{} ... complete", filename);
		}
		catch (std::exception err) {
			LOG_ERR_("failure to load asset: {}", err.what());
			good = false;
		}
	}

	//LOG_INFO("Deserializing complete: {}ms", timer.getElapsedTime().asMilliseconds());
	return good;
}

////////////////////////////////////////////////////////////

template <typename  AssetType, typename  FlatType = typename flat_type<AssetType>::type>
auto buildPackAssets(flatbuffers::FlatBufferBuilder& builder, AssetMap<AssetType>& map) {
	std::vector<flatbuffers::Offset<FlatType>> assetVec;
	for (auto& asset : map) {
		std::string info = "\t" + asset.second->getAssetName();
		log::scope scope;
		try {
			assetVec.push_back(asset.second->writeToFlat(builder));
			LOG_INFO("{} ... complete", asset.second->getAssetName());
		}
		catch (std::exception err) {
			LOG_ERR_("{} ...  pack failed: {}", asset.second->getAssetName(), err.what());
		}
	}
	return builder.CreateVector(assetVec);
}

bool Resources::buildPackFile(const std::string& packFilename) {

	LOG_INFO("Serializing resource assets");
	//sf::Clock timer;
	flatbuffers::FlatBufferBuilder builder;

	using namespace flat::resources;

	auto sprvec  = buildPackAssets<SpriteAsset >(builder, resource.sprites);
	auto tilevec = buildPackAssets<TilesetAsset>(builder, resource.tilesets);
	auto lvlvec  = buildPackAssets<LevelAsset  >(builder, resource.levels);

	ResourcesFBuilder resBuilder(builder);
	resBuilder.add_sprites(sprvec);
	resBuilder.add_tilesets(tilevec);
	resBuilder.add_levels(lvlvec);

	auto res = resBuilder.Finish();
	builder.Finish(res);
	//LOG_INFO("Serializing complete: {}ms", timer.getElapsedTime().asMilliseconds());

	LOG_INFO("Writing serialized data to file: {}", packFilename);
	std::fstream file(DEBUGPATH + packFilename, std::ios::binary | std::ios_base::out);
	if (file) {

		uint8_t* data = builder.GetBufferPointer();
		uint8_t* end = builder.GetBufferPointer() + builder.GetSize();

		while (data < end) {

			file.write(
				(char*)data,
				std::min(
					128ll,
					static_cast<long long>(end - data)
				)
			);
			data += 128;
		}
		file.close();
	}
	LOG_INFO("Writing complete");

	return true;
}

////////////////////////////////////////////////////////////

// indexfile parsing
std::vector<std::string> parseDataNodes(xml_node<>* first_node, const char* type);

template<class Type>
bool loadAssets(const std::string& path, std::vector<std::string>& names) {

	for (const auto& asset : names) {
		std::string info = "\t" + path + asset + " ... ";
		std::unique_ptr<Type> ptr = std::make_unique<Type>(asset);
		log::scope scope;
		if (ptr->loadFromFile(path)) {
			Resources::add(asset, std::move(ptr));
			LOG_INFO("{}{} ... complete", path, asset);
		}
		else {
			//std::cout << "failed to load: " << typeid(*ptr.get()).name() << ", " << asset << std::endl;
			LOG_ERR_("{}{} ... failed to load: {}, {1}", path, asset, typeid(*ptr.get()).name());
			return false;
		}
	}
	return true;
}


bool loadFromIndex(const std::string& indexFile) {
	//using namespace rapidxml;

	bool r = true;

	std::string spriteRelPath;
	std::vector<std::string> spriteNames;

	std::string tilesetRelPath;
	std::vector<std::string> tilesetNames;

	std::string levelRelPath;
	std::vector<std::string> levelNames;

	std::string dataPath = std::string(DEBUGPATH) + "data/";

	// try to open the index file
	std::ifstream ndxStream(dataPath + indexFile, std::ios::binary | std::ios::ate);
	if (ndxStream.is_open()) {
		char* datatypePath;

		//parse index file
		std::streamsize len = ndxStream.tellg();
		ndxStream.seekg(0, std::ios::beg);

		char* indexContent = new char[len + 1];

		ndxStream.read(indexContent, len);
		indexContent[len] = '\0';
		ndxStream.close();

		xml_document<>* doc = new xml_document<>();
		try {
			doc->parse<0>(indexContent);

			xml_node<>* index = doc->first_node("index");

			if (index) {

				xml_node<>* datatype = index->first_node();

				while (datatype) {
					char* name = datatype->name();
					datatypePath = datatype->first_attribute("path")->value();

					if (strcmp(name, "sprites") == 0) {
						spriteRelPath = dataPath + datatypePath;
						spriteNames = parseDataNodes(datatype->first_node(), "sprite");
					}
					else if (strcmp(name, "tilesets") == 0) {
						tilesetRelPath = dataPath + datatypePath;
						tilesetNames = parseDataNodes(datatype->first_node(), "tileset");
					}
					else if (strcmp(name, "levels") == 0) {
						levelRelPath = dataPath + datatypePath;
						levelNames = parseDataNodes(datatype->first_node(), "level");
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
		delete doc;
		delete[] indexContent;

		//LOG_INFO("Index file closed");
	}
	else {
		r = false;
	}

	LOG_INFO("Loading assets");
	//sf::Clock timer;

	// load sprites
	r &= loadAssets<SpriteAsset>(spriteRelPath, spriteNames);

	//loadAnimations();

	// load tilesets
	r &= loadAssets<TilesetAsset>(tilesetRelPath, tilesetNames);

	// load level, requires tilesets to be loaded first
	r &= loadAssets<LevelAsset>(levelRelPath, levelNames);

	//LOG_INFO("Loading assets complete: {}ms", timer.getElapsedTime().asMilliseconds());

	return r;
}

std::vector<std::string> parseDataNodes(xml_node<>* first_node, const char* type) {

	xml_node<>* node = first_node;
	std::vector<std::string> names;

	while (node) {
		if (strcmp(node->name(), type) == 0) {
			names.push_back(node->value());
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
	assert(resource.loadMethod != AssetSource::PACK_FILE);

	for (auto& [key, val] : resource.sprites) {
		ResourceWatcher::add_watch(val->getFilePath() + val->getAssetName() + spriteExt, val.get());
		ResourceWatcher::add_watch(val->getTexPath(), val.get());
	}
	for (auto& [key, val] : resource.tilesets) {
		ResourceWatcher::add_watch(val->getFilePath() + val->getAssetName() + tilesetExt, val.get());
		ResourceWatcher::add_watch(val->getTexPath(), val.get());
	}
	for (auto& [key, val] : resource.levels) {
		ResourceWatcher::add_watch(val->getFilePath() + val->getAssetName() + levelExt, val.get());
	}
}

}