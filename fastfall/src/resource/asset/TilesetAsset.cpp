#include "fastfall/resource/asset/TilesetAsset.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/game/level/TileLogic.hpp"

#include "fastfall/util/xml.hpp"
#include "fastfall/util/cardinal.hpp"
#include "fastfall/util/log.hpp"


#include <assert.h>
#include <fstream>

//#include <ranges>



namespace ff {



struct TileState {
	TilesetAsset* owner;
	Tile* tile;
	TileLogicData logic;
};

std::map<std::string, void(*)(TileState&, char*)> tileProperties
{
	{"shape", [](TileState& state, char* value)
	{
		state.tile->shape = TileShape(value);
	}},

	{"logic", [](TileState& state, char* value)
	{
		state.logic.logicType = value;
	}},
	{"logic_arg", [](TileState& state, char* value)
	{
		state.logic.logicArg = value;
	}},
	{"next_x", [](TileState& state, char* value)
	{
		state.tile->next_offset.x = std::stoi(value);

	}},
	{"next_y", [](TileState& state, char* value)
	{
		state.tile->next_offset.y = std::stoi(value);

	}},
	{"next_tileset", [](TileState& state, char* value)
	{
		if (strlen(value) > 0 && strcmp(state.owner->getAssetName().c_str(), value) != 0) {
			state.tile->next_tileset = state.owner->getTilesetRefIndex(value);
		}
	}},
	{"material", [](TileState& state, char* value)
	{
		state.tile->material = getTileMaterial(value);
	}},
	{"material_facing", [](TileState& state, char* value)
	{
		if (strcmp(value, "north")) {
			state.tile->matFacing = Cardinal::NORTH;
		}
		else if (strcmp(value, "east")) {
			state.tile->matFacing = Cardinal::EAST;
		}
		else if (strcmp(value, "south")) {
			state.tile->matFacing = Cardinal::SOUTH;
		}
		else if (strcmp(value, "west")) {
			state.tile->matFacing = Cardinal::WEST;
		}
	}}

};


TilesetAsset::TilesetAsset(const std::string& filename) :
	TextureAsset(filename)
{

}

void TilesetAsset::parseTileProperties(xml_node<>* propsNode, Tile& t) {
	xml_node<>* propNode = propsNode->first_node();

	TileState state;
	state.owner = this;
	state.tile = &t;

	//TileLogicData logic;

	while (propNode) {
		if (strcmp("property", propNode->name()) != 0)
			throw parse_error("not a property", nullptr);

		char* name = propNode->first_attribute("name")->value();
		char* value = propNode->first_attribute("value")->value();

		auto prop = tileProperties.find(name);
		if (prop != tileProperties.end()) {
			prop->second(state, value);
		}
		else {
			LOG_WARN("unknown tile property: {} = {}", name, value);
		}

		propNode = propNode->next_sibling();
	}

	if (!state.logic.logicType.empty()) {
		setTileLogic(t.pos, state.logic);
	}
}


int TilesetAsset::getTilesetRefIndex(std::string_view tileset_name) {
	auto r = std::find(tilesetRef.begin(), tilesetRef.end(), tileset_name);
	if (r != tilesetRef.end()) {
		return std::distance(tilesetRef.begin(), r);
	}
	else {
		tilesetRef.push_back(tileset_name.data());
		return tilesetRef.size() - 1;
	}
}

bool TilesetAsset::loadFromFile(const std::string& relpath) {

	bool texLoaded = false;

	assetFilePath = relpath;

	bool r = true;
	std::unique_ptr<char[]> charPtr = readXML(assetFilePath + assetName + tilesetExt);

	if (charPtr) {
		char* xmlContent = charPtr.get();

		xml_document<>* doc = new xml_document<>();
		try {
			doc->parse<0>(xmlContent);

			xml_node<>* tilesetNode = doc->first_node("tileset");
			if (!tilesetNode)
				throw parse_error("no tileset node", nullptr);

			int columns = atoi(tilesetNode->first_attribute("columns")->value());
			if (columns == 0)
				throw parse_error("columns == 0", nullptr);

			xml_node<>* imgNode = tilesetNode->first_node("image");
			if (!imgNode)
				throw parse_error("no image node", nullptr);

			std::string source = imgNode->first_attribute("source")->value();
			fullpath = relpath + source;
			if (!tex.loadFromFile(fullpath))
				throw parse_error("could not load sprite source", nullptr);

			texTileSize = Vec2u(tex.size()) / TILESIZE;

			//assert(texTileSize.x <= Vector2s::MAX);

			// parse tiles
			xml_node<>* tileNode = tilesetNode->first_node("tile");
			while (tileNode) {
				Tile t;

				int id = atoi(tileNode->first_attribute("id")->value());

				t.pos = Vec2u(id % columns, id / columns);
				t.origin = this;

				xml_node<>* propNode = tileNode->first_node("properties");
				parseTileProperties(propNode, t);

				tileData.insert(std::make_pair(t.pos, t));
				tileNode = tileNode->next_sibling();
			}
		}
		catch (parse_error& err) {
			std::cout << assetName << ": " << err.what() << std::endl;
			r = false;
		}
		delete doc;
	}
	else {
		std::cout << "Could not open file: " << relpath + assetName + tilesetExt << std::endl;
		r = false;
	}

	loaded = r;
	return r;
}


bool TilesetAsset::loadFromFlat(const flat::resources::TilesetAssetF* builder) {

	assetName = builder->name()->c_str();
	texTileSize = Vec2u(builder->tileSize()->x(), builder->tileSize()->y());

	tileData.clear();
	tilesetRef.clear();
	logicData.clear();

	for (auto tileR : *builder->tilesetRefs()) {
		tilesetRef.push_back(tileR->str());
	}

	for (auto logicR : *builder->tilesetLogics()) {
		Vec2u pos{ logicR->pos()->x(), logicR->pos()->y() };
		logicData.insert(std::make_pair(pos, 
			TileLogicData{
				.logicType = logicR->logic()->str(), 
				.logicArg = logicR->logic_arg()->str()
			}
		));
	}

	for (auto tileT = builder->tileData()->begin(); tileT != builder->tileData()->end(); tileT++) {
		Tile tile;
		tile.origin = this;
		tile.pos = Vec2u(tileT->pos().x(), tileT->pos().y());
		tile.shape.type = static_cast<TileShape::Type>(tileT->shape().type());
		tile.shape.shapeTouches = tileT->shape().shapeTouches();
		tile.shape.hflipped = tileT->shape().hflip();
		tile.shape.vflipped = tileT->shape().vflip();
		tile.next_offset = Vec2i(tileT->next_offset().x(), tileT->next_offset().y());
		tile.next_tileset = tileT->next_tilesetNdx();
		tileData.insert(std::make_pair(tile.pos, tile));
	}

	loaded = tex.loadFromStream(builder->image()->Data(), builder->image()->size());
	return loaded;
}

flatbuffers::Offset<flat::resources::TilesetAssetF> TilesetAsset::writeToFlat(flatbuffers::FlatBufferBuilder& builder) const {
	using namespace flat::resources;
	using namespace flat::math;

	std::vector<flatbuffers::Offset<TilesetLogicF>> logicdata_vec;
	for (const auto& [position, data] : logicData) {
		TilesetLogicFBuilder logics(builder);
		Vec2Fu pos{ position.x, position.y };

		logics.add_pos(&pos);
		logics.add_logic(builder.CreateString(data.logicType));
		logics.add_logic_arg(builder.CreateString(data.logicArg));
		logicdata_vec.push_back(logics.Finish());
	}
	auto logicdata = builder.CreateVector(logicdata_vec);

	auto flat_refs = builder.CreateVectorOfStrings(tilesetRef);

	std::vector<TileF> tiledata;
	for (const auto& tile : tileData) {
		Vec2Fu pos{ tile.second.pos.x, tile.second.pos.y };

		TileShapeF shape(
			static_cast<uint32_t>(tile.second.shape.type),
			tile.second.shape.shapeTouches,
			tile.second.shape.hflipped,
			tile.second.shape.vflipped
		);

		Vec2Fi next{ tile.second.next_offset.x, tile.second.next_offset.y };

		TileF t{ pos, shape, next, tile.second.next_tileset };
		tiledata.push_back(t);

	}
	auto flat_tiledata = builder.CreateVectorOfStructs(tiledata);


	auto flat_assetName = builder.CreateString(assetName);

	assert(!fullpath.empty());
	std::vector<int8_t> texData = readFile(fullpath.c_str());
	auto flat_texdata = builder.CreateVector(texData);

	Vec2Fu flat_tileSize(texTileSize.x, texTileSize.y);

	TilesetAssetFBuilder tileBuilder(builder);
	tileBuilder.add_name(flat_assetName);
	tileBuilder.add_image(flat_texdata);
	tileBuilder.add_tileSize(&flat_tileSize);
	tileBuilder.add_tileData(flat_tiledata);
	tileBuilder.add_tilesetRefs(flat_refs);
	tileBuilder.add_tilesetLogics(logicdata);
	return tileBuilder.Finish();
}

Tile TilesetAsset::getTile(Vec2u texPos) const {
	// assert this is actually on our texture
	assert(texPos.x < texTileSize.x&& texPos.y < texTileSize.y);

	auto r = tileData.find(texPos);

	if (r != tileData.end()) {
		return r->second;
	}
	else {
		// create a default, empty tile here
		auto t = Tile();
		t.origin = this;
		t.pos = texPos;
		t.shape.type = TileShape::Type::EMPTY;
		return t;
	}
}


void TilesetAsset::ImGui_getContent() {
	TextureAsset::ImGui_getContent();
}

}