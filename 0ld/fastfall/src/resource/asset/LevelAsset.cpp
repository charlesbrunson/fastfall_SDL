#include "fastfall/resource/asset/LevelAsset.hpp"
#include "fastfall/resource/Resources.hpp"

#include "leveltypes/TileTMX.hpp"
#include "leveltypes/TilesetTMX.hpp"
#include "leveltypes/ObjectTMX.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/util/xml.hpp"

#include "fastfall/engine/config.hpp"

#include <sstream>
#include <functional>
#include <string>

namespace ff {

LevelAsset::LevelAsset(const std::filesystem::path& t_asset_path) :
	Asset(t_asset_path)
{

}

// TMX Parsing, LoadFromFile()
/////////////////////////////////////////////////////////////

struct LevelProperties {
	Vec2u size;
	Color bgColor;
};



LevelProperties tmxParseLevelProperties(xml_node<>* mapNode) {

	xml_attribute<>* mapAttr = mapNode->first_attribute();

	const static std::map<std::string, void(*)(LevelProperties&, char*)> validLevelProps
	{
		{"tilewidth", [](LevelProperties& prop, char* val) {
			assert(atoi(val) == TILESIZE);
		}},
		{"tileheight", [](LevelProperties& prop, char* val) {
			assert(atoi(val) == TILESIZE);
		}},
		{"width", [](LevelProperties& prop, char* val) {
			prop.size.x = atoi(val);
		}},
		{"height", [](LevelProperties& prop, char* val) {
			prop.size.y = atoi(val);
		}},
		{"backgroundcolor", [](LevelProperties& prop, char* val) {
			// get hex code for color, appending ff for alpha
			char* cptr = &val[1];
			unsigned int colorhex;
			std::stringstream ss;
			ss << std::hex << cptr << "ff";
			ss >> colorhex;

			prop.bgColor = Color(colorhex);
		}}
	};
	LevelProperties prop;

	// builtin map attributes
	while (mapAttr) {
		char* attrName = mapAttr->name();
		char* attrVal = mapAttr->value();

		auto it = validLevelProps.find(attrName);
		if (it != validLevelProps.end()) {
			it->second(prop, attrVal);
		}

		mapAttr = mapAttr->next_attribute();
	}

	// now custom properties
	xml_node<>* properties = mapNode->first_node("properties");
	if (properties) {
		xml_node<>* property = properties->first_node("property");
		while (property) {
			char* propName = property->first_attribute("name")->value();
			char* propValue = property->first_attribute("value")->value();

			auto it = validLevelProps.find(propName);
			if (it != validLevelProps.end()) {
				it->second(prop, propValue);
			}

			property = property->next_sibling();
		}
	}


	assert(prop.size.x > 0);
	assert(prop.size.y > 0);
	return prop;
}

/////////////////////////////////////////////////////////////

bool LevelAsset::loadFromFile() {

	backgroundColor = Color::White;
	lvlTileSize = Vec2u(0u, 0u);

	bool hasActive = false;

	bool r = true;

	std::unique_ptr<char[]> charPtr = readXML(asset_path);
	if (charPtr) {
		char* xmlContent = charPtr.get();
		
		auto doc = std::make_unique<xml_document<>>();

		try {
			doc->parse<0>(xmlContent);
			xml_node<>* mapNode = doc->first_node("map");
			if (!mapNode)
				throw parse_error("no map node", nullptr);

			//parse level properties
			LevelProperties lvlprop = tmxParseLevelProperties(mapNode);
			lvlTileSize = lvlprop.size;
			backgroundColor = lvlprop.bgColor;

			// parse tilesets
			TilesetMap tilesetDeps = TilesetTMX::parse(mapNode->first_node("tileset"));


			// parse layers
			bool hasObjectLayer = false;
			auto layerNode = mapNode->first_node();
			while (layerNode) {
				//LOG_INFO("node: {}", layerNode->name());
				if (strcmp(layerNode->name(), "layer") == 0) 
				{
					if (!hasObjectLayer)
					{
						layers.push_bg_front(TileLayerData::loadFromTMX(layerNode, tilesetDeps));
					}
					else
					{
						layers.push_fg_front(TileLayerData::loadFromTMX(layerNode, tilesetDeps));
					}
					assert(layers.get_tile_layers().back().tilelayer.getSize() == lvlTileSize);
				}
				else if (strcmp(layerNode->name(), "objectgroup") == 0) 
				{
					assert(!hasObjectLayer);
					hasObjectLayer = true;
					layers.get_obj_layer() = ObjectTMX::parse(layerNode);
				}
				layerNode = layerNode->next_sibling();
			}
			assert(hasObjectLayer);
		}
		catch (parse_error& err) {
			std::cout << asset_path << ": " << err.what() << std::endl;
			r = false;
		}
	}
	else {
		std::cout << "Could not open file: " << asset_path << std::endl;
		r = false;
	}

	loaded = r;
	return true;
}

bool LevelAsset::reloadFromFile() {

	bool loaded = false;

	LevelAsset n_level{ asset_path };

	try {
		if (loaded = n_level.loadFromFile(); loaded) {
			*this = std::move(n_level);
		}
	}
	catch (std::exception err)
	{
		LOG_ERR_("failed to reload level asset: {}", err.what());
	}
	return loaded;
}

void LevelAsset::ImGui_getContent(secs deltaTime) {
	ImGui::Text("[%3u, %3u] %s", lvlTileSize.x, lvlTileSize.y, asset_name.c_str());
}


}
