#include "fastfall/resource/asset/LevelAsset.hpp"
#include "fastfall/resource/Resources.hpp"

#include "leveltypes/TileTMX.hpp"
#include "leveltypes/TilesetTMX.hpp"
#include "leveltypes/ObjectTMX.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/util/xml.hpp"
#include "fastfall/util/base64.hpp"
#include "fastfall/util/cardinal.hpp"

#include "fastfall/engine/config.hpp"

#include "fastfall/game/Instance.hpp"
#include "fastfall/game/level/LevelEditor.hpp"

#include <sstream>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <string>

namespace ff {

LevelAsset::LevelAsset(const std::string& filename) :
	Asset(filename)
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

bool LevelAsset::loadFromFile(const std::string& relpath) {

	backgroundColor = Color::White;
	lvlTileSize = Vec2u(0u, 0u);

	bool hasActive = false;

	bool r = true;

	assetFilePath = relpath;
	std::unique_ptr<char[]> charPtr = readXML(assetFilePath + assetName + levelExt);
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
			tilesetDeps = TilesetTMX::parse(mapNode->first_node("tileset"));

			bool hasObjectLayer = false;

			// parse layers
			auto layerNode = mapNode->first_node();
			while (layerNode) {
				if (strcmp(layerNode->name(), "layer") == 0) {

					layers.push_back(TileTMX::parse(layerNode, tilesetDeps));
					assert(std::get<TileLayerRef>(layers.back().layer).tileSize == lvlTileSize);
				}
				else if (strcmp(layerNode->name(), "objectgroup") == 0) {
					assert(!hasObjectLayer);
					hasObjectLayer = true;

					layers.push_back(ObjectTMX::parse(layerNode));
				}

				layerNode = layerNode->next_sibling();
			}
			assert(hasObjectLayer);
		}
		catch (parse_error& err) {
			std::cout << assetName << ": " << err.what() << std::endl;
			r = false;
		}
	}
	else {
		std::cout << "Could not open file: " << relpath + assetName + levelExt << std::endl;
		r = false;
	}

	loaded = r;
	return true;
}

bool LevelAsset::reloadFromFile() {

	bool loaded = false;

	LevelAsset n_level{ getAssetName() };

	if (n_level.loadFromFile(assetFilePath)) {
		*this = std::move(n_level);
		loaded = true;


		for (auto& [id, inst] : AllInstances())
		{
			Level* active = inst.getActiveLevel();
			for (auto& [name, lvl_uptr] : inst.getAllLevels())
			{
				if (lvl_uptr.get() == active) {
					LevelEditor edit(*lvl_uptr.get(), false);
					LOG_INFO("Applying reloaded level asset to active level");
					edit.applyLevelAsset(this);
					inst.populateSceneFromLevel(*lvl_uptr);
				}
				else {
					LOG_INFO("Reinit inactive level with reloaded level asset");
					lvl_uptr->init(*this);
				}
			}
		}
	}
	return loaded;
}




bool LevelAsset::loadFromFlat(const flat::resources::LevelAssetF* builder) {


	/*
	using namespace flat::resources;
	using namespace flat::math;

	bool hasObjectLayer = false;

	layers.clear();
	tilesetDeps.clear();

	assetName = builder->name()->c_str();
	backgroundColor = Color(
		builder->bgColor()->r(),
		builder->bgColor()->g(),
		builder->bgColor()->b(),
		builder->bgColor()->a());
	lvlTileSize = Vec2u(builder->lvlSize()->x(), builder->lvlSize()->y());
	borderCardinalBits = builder->borders();

	for (auto deps = builder->tilesetDeps()->begin(); deps != builder->tilesetDeps()->end(); deps++) {
		tilesetDeps.insert(std::make_pair(deps->gid(), deps->tilesetName()->c_str()));
	}

	for (auto layerT = builder->layers()->begin(); layerT != builder->layers()->end(); layerT++) {
		if (layerT->layer_type() == AnyLayerF::AnyLayerF_TileLayerF) {
			auto tileT = layerT->layer_as_TileLayerF();

			LayerRef layerRef(LayerType::TILELAYER);
			layerRef.id = layerT->id();

			layerRef.tileLayer->tileSize = Vec2u(tileT->tileSize()->x(), tileT->tileSize()->y());
			layerRef.tileLayer->innerSize = Vec2u(tileT->innerSize()->x(), tileT->innerSize()->y());
			layerRef.tileLayer->has_parallax = tileT->has_parallax();
			layerRef.tileLayer->scrollrate = Vec2f(tileT->scrollrate()->x(), tileT->scrollrate()->y());

			for (auto tT = tileT->tiles()->begin(); tT != tileT->tiles()->end(); tT++) {
				Vec2u pos(tT->tilePos().x(), tT->tilePos().y());

				TileRef tileRef;
				tileRef.tile_id = tT->gid();
				tileRef.texPos = Vec2u(tT->texPos().x(), tT->texPos().y());

				auto it = tilesetDeps.begin();
				auto next = tilesetDeps.begin();

				for (auto it = tilesetDeps.cbegin(); it != tilesetDeps.cend(); it++) {
					if (tileRef.tile_id >= it->first) {
						tileRef.tilesetName = &it->second;
					}
					else {
						break;
					}
				}

				layerRef.tileLayer->tiles.insert(std::make_pair(pos, tileRef));
			}

			layers.push_back(std::move(layerRef));
		}
		else if (layerT->layer_type() == AnyLayerF::AnyLayerF_ObjectLayerF) {
			auto objT = layerT->layer_as_ObjectLayerF();

			LayerRef layerRef(LayerType::OBJECTLAYER);
			layerRef.id = layerT->id();

			for (auto tT = objT->objects()->begin(); tT != objT->objects()->end(); tT++) {

				ObjectRef obj(layerRef.objLayer.get());
				obj.id = tT->id();
				obj.name = tT->name()->c_str();
				obj.position = Vec2i(tT->pos()->x(), tT->pos()->y());
				obj.type = tT->type_hash();
				obj.width = tT->width();
				obj.height = tT->height();

				for (auto prop = tT->properties()->begin(); prop != tT->properties()->end(); prop++) {
					obj.properties.push_back(std::make_pair(prop->propertyName()->c_str(), prop->propertyValue()->c_str()));
				}

				for (auto p = tT->points()->begin(); p != tT->points()->end(); p++) {
					obj.points.push_back(Vec2i(p->x(), p->y()));
				}

				layerRef.objLayer->objects.insert(std::make_pair(obj.id, obj));


			}
			hasObjectLayer = true;
			layers.push_back(std::move(layerRef));
		}
	}

	if (!hasObjectLayer)
		throw std::runtime_error("Level has no object layer");

	loaded = true;
	return true;
	*/
	return false;
}

flatbuffers::Offset<flat::resources::LevelAssetF> LevelAsset::writeToFlat(flatbuffers::FlatBufferBuilder& builder) const {

	/*
	using namespace flat::resources;
	using namespace flat::math;

	auto flat_assetName = builder.CreateString(assetName);
	ColorF flat_bg(
		static_cast<uint8_t>(backgroundColor.r),
		static_cast<uint8_t>(backgroundColor.g),
		static_cast<uint8_t>(backgroundColor.b),
		static_cast<uint8_t>(backgroundColor.a)
	);
	Vec2Fu flat_lvlsize(lvlTileSize.x, lvlTileSize.y);


	std::vector<flatbuffers::Offset<LevelTilesetDepF>> _tilesetDeps;
	for (const auto& tDep : tilesetDeps) {

		auto tilesetName = builder.CreateString(tDep.second);

		LevelTilesetDepFBuilder dep(builder);
		dep.add_gid(tDep.first);
		dep.add_tilesetName(tilesetName);
		_tilesetDeps.push_back(dep.Finish());
	}
	auto flat_tilesetDeps = builder.CreateVector(_tilesetDeps);

	std::vector<flatbuffers::Offset<LevelLayerF>> _layers;
	for (const auto& lay : layers) {

		flatbuffers::Offset<void> flat_layerref;
		if (lay.type == LayerType::TILELAYER) {

			TileLayerRef* tilelayer = lay.tileLayer.get();


			Vec2Fu flat_layerTileInternalSize(tilelayer->innerSize.x, tilelayer->innerSize.y);
			Vec2Fu flat_layerTileSize(tilelayer->tileSize.x, tilelayer->tileSize.y);
			Vec2Ff flat_scrollrate(tilelayer->scrollrate.x, tilelayer->scrollrate.y);

			std::vector<TileRefF> tiles;

			unsigned tilesetCounter = 0;
			std::map<const std::string*, unsigned int> tilesetNameSet;
			std::vector<std::string> tilesetNames;

			for (const auto& tile : tilelayer->tiles) {

				unsigned tilesetID = -1;
				auto r = tilesetNameSet.find(tile.second.tilesetName);
				if (r != tilesetNameSet.end()) {

					tilesetID = r->second;
				}
				else {
					tilesetNameSet.insert(std::make_pair(tile.second.tilesetName, tilesetCounter));
					tilesetNames.push_back(*tile.second.tilesetName);
					tilesetID = tilesetCounter;
					tilesetCounter++;
				}

				TileRefF tileref(
					tile.second.tile_id,
					Vec2Fu(tile.first.x, tile.first.y),
					Vec2Fu(tile.second.texPos.x, tile.second.texPos.y),
					tilesetID
				);

				tiles.push_back(tileref);

			}
			auto flat_tiles = builder.CreateVectorOfStructs(tiles);
			auto flat_tilesetNames = builder.CreateVectorOfStrings(tilesetNames);

			TileLayerFBuilder tileBuilder(builder);
			tileBuilder.add_has_parallax(tilelayer->has_parallax);
			tileBuilder.add_innerSize(&flat_layerTileInternalSize);
			tileBuilder.add_tileSize(&flat_layerTileSize);
			tileBuilder.add_scrollrate(&flat_scrollrate);
			tileBuilder.add_tiles(flat_tiles);
			tileBuilder.add_tilesetsReq(flat_tilesetNames);
			flat_layerref = tileBuilder.Finish().Union();
		}
		else if (lay.type == LayerType::OBJECTLAYER) {

			ObjectLayerRef* objlayer = lay.objLayer.get();

			std::vector<flatbuffers::Offset<ObjectF>> objects;
			for (auto& obj : objlayer->objects) {


				auto flat_objname = builder.CreateString(obj.second.name);
				Vec2Fi flat_pos(obj.second.position.x, obj.second.position.y);

				std::vector<flatbuffers::Offset<PropertyF>> objectsProps;
				for (auto& props : obj.second.properties) {

					objectsProps.push_back(
						CreatePropertyF(builder,
							builder.CreateString(props.first),
							builder.CreateString(props.second)
						)
					);
				}
				auto flat_objectsProps = builder.CreateVector(objectsProps);

				std::vector<Vec2Fi> objectPoints;
				for (auto& point : obj.second.points) {
					objectPoints.push_back(Vec2Fi(point.x, point.y));
				}
				auto flat_objectsPoints = builder.CreateVectorOfStructs(objectPoints);

				ObjectFBuilder objBuilder(builder);
				objBuilder.add_id(obj.first);
				objBuilder.add_name(flat_objname);
				objBuilder.add_type_hash(obj.second.type);
				objBuilder.add_pos(&flat_pos);
				objBuilder.add_width(obj.second.width);
				objBuilder.add_height(obj.second.height);
				objBuilder.add_properties(flat_objectsProps);
				objBuilder.add_points(flat_objectsPoints);
				objects.push_back(objBuilder.Finish());
			}
			auto flat_objects = builder.CreateVector(objects);

			ObjectLayerFBuilder objBuilder(builder);
			objBuilder.add_objects(flat_objects);
			flat_layerref = objBuilder.Finish().Union();
		}

		LevelLayerFBuilder flat_layer(builder);
		flat_layer.add_id(lay.id);
		if (lay.type == LayerType::TILELAYER) {
			flat_layer.add_layer_type(AnyLayerF_TileLayerF);
		}
		else if (lay.type == LayerType::OBJECTLAYER) {
			flat_layer.add_layer_type(AnyLayerF_ObjectLayerF);
		}
		flat_layer.add_layer(flat_layerref);
		_layers.push_back(flat_layer.Finish());
	}
	auto flat_layers = builder.CreateVector(_layers);


	LevelAssetFBuilder lvlBuilder(builder);
	lvlBuilder.add_name(flat_assetName);
	lvlBuilder.add_bgColor(&flat_bg);
	lvlBuilder.add_lvlSize(&flat_lvlsize);
	lvlBuilder.add_layers(flat_layers);
	lvlBuilder.add_tilesetDeps(flat_tilesetDeps);
	lvlBuilder.add_borders(borderCardinalBits);
	return lvlBuilder.Finish();
	*/
	return false;
}

void LevelAsset::ImGui_getContent() {
	ImGui::Text("[%3u, %3u] %s", lvlTileSize.x, lvlTileSize.y, getAssetName().c_str());
}


}
