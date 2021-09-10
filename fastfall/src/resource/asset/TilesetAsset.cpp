#include "fastfall/resource/asset/TilesetAsset.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/game/level/TileLogic.hpp"

#include "fastfall/util/xml.hpp"
#include "fastfall/util/cardinal.hpp"
#include "fastfall/util/log.hpp"


#include <assert.h>
#include <fstream>

namespace ff {

const std::map<std::string, void(*)(TilesetAsset&, TilesetAsset::TileData&, char*)> TilesetAsset::tileProperties
{
	{"shape", [](TilesetAsset& asset, TileData& state, char* value)
	{
		state.tile.shape = TileShape(value);
	}},

	{"logic", [](TilesetAsset& asset, TileData& state, char* value)
	{
		state.has_prop_bits |= TileHasProp::HasLogic;
		state.tileLogicNdx = asset.addLogicType(value);

	}},
	{"logic_arg", [](TilesetAsset& asset, TileData& state, char* value)
	{
		if (state.has_prop_bits & TileHasProp::HasLogic)
		{
			state.has_prop_bits |= TileHasProp::HasLogicArgs;
			state.tileLogicParamNdx = asset.addLogicArgs(state.tileLogicNdx, value);
		}
		else 
		{
			LOG_ERR_("Tileset: {}, unknown logic type for args at {}", asset.getAssetName(), state.tile.pos.to_string());
		}
	}},
	{"next_x", [](TilesetAsset& asset, TileData& state, char* value)
	{
		state.tile.next_offset.x = std::stoi(value);
	}},
	{"next_y", [](TilesetAsset& asset, TileData& state, char* value)
	{
		state.tile.next_offset.y = std::stoi(value);
	}},
	{"next_tileset", [](TilesetAsset& asset, TileData& state, char* value)
	{
		if (strlen(value) > 0 && strcmp(asset.getAssetName().c_str(), value) != 0) {

			state.tile.next_tileset = asset.getTilesetRefIndex(value);
		}
	}},
	{"material", [](TilesetAsset& asset, TileData& state, char* value)
	{
		state.has_prop_bits |= TileHasProp::HasMaterial;
		state.tileMatNdx = asset.addMaterial(value);
	}},
	{"material_facing", [](TilesetAsset& asset, TileData& state, char* value)
	{
		if (strcmp(value, "north")) {
			state.tile.matFacing = Cardinal::NORTH;
		}
		else if (strcmp(value, "east")) {
			state.tile.matFacing = Cardinal::EAST;
		}
		else if (strcmp(value, "south")) {
			state.tile.matFacing = Cardinal::SOUTH;
		}
		else if (strcmp(value, "west")) {
			state.tile.matFacing = Cardinal::WEST;
		}
	}}

};


TilesetAsset::TilesetAsset(const std::string& filename) :
	TextureAsset(filename)
{
}

unsigned TilesetAsset::addLogicType(std::string type)
{
	unsigned count = 0;
	auto logic_it = std::find_if(tileLogic.begin(), tileLogic.end(),
		[&type, &count](const TilesetLogic& t_logic) {
			if (t_logic.logicType != type) {
				count++;
			}
			return t_logic.logicType == type;
		});

	if (logic_it == tileLogic.end()) {
		count = tileLogic.size();
		tileLogic.push_back({ type, {} });
	}
	return count;
}
unsigned TilesetAsset::addLogicArgs(unsigned logicType, std::string args)
{
	unsigned count = 0;
	auto& logicArgs = tileLogic.at(logicType).logicArg;

	auto args_it = std::find_if(logicArgs.begin(), logicArgs.end(),
		[&args, &count](const std::string& t_args) {
			if (t_args != args) {
				count++;
			}
			return t_args == args;
		});

	if (args_it == logicArgs.end()) {
		count = logicArgs.size();
		logicArgs.push_back(args);
	}
	return count;
}
unsigned TilesetAsset::addMaterial(std::string mat)
{
	unsigned count = 0;
	auto mat_it = std::find_if(tileMat.begin(), tileMat.end(),
		[&mat, &count](const std::string& t_mat) {
			if (t_mat != mat) {
				count++;
			}
			return t_mat == mat;
		});

	if (mat_it == tileMat.end()) {
		count = tileMat.size();
		tileMat.push_back(mat);
	}
	return count;
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

void TilesetAsset::loadFromFile_TileProperties(xml_node<>* propsNode, TileData& t) {
	xml_node<>* propNode = propsNode->first_node();
	while (propNode) {
		if (strcmp("property", propNode->name()) != 0)
			throw parse_error("not a property", nullptr);

		char* name = propNode->first_attribute("name")->value();
		char* value = propNode->first_attribute("value")->value();

		auto prop = tileProperties.find(name);
		if (prop != tileProperties.end()) {
			prop->second(*this, t, value);
		}
		else {
			LOG_WARN("unknown tile property: {} = {}", name, value);
		}

		propNode = propNode->next_sibling();
	}
}


void TilesetAsset::loadFromFile_Header(xml_node<>* tileset_node, const std::string_view& relpath)
{
	int columns = atoi(tileset_node->first_attribute("columns")->value());
	if (columns == 0)
		throw parse_error("columns == 0", nullptr);

	xml_node<>* imgNode = tileset_node->first_node("image");
	if (!imgNode)
		throw parse_error("no image node", nullptr);

	std::string source = imgNode->first_attribute("source")->value();
	fullpath = std::string(relpath) + source;
	if (!tex.loadFromFile(fullpath))
		throw parse_error("could not load sprite source", nullptr);

	texTileSize = Vec2u(tex.size()) / TILESIZE;

}

void TilesetAsset::loadFromFile_Tile(xml_node<>* tile_node)
{
	int id = atoi(tile_node->first_attribute("id")->value());

	TileData& t = tiles[id];
	t.has_prop_bits |= TileHasProp::HasTile;
	t.tile.pos = Vec2u{ id % texTileSize.x, id / texTileSize.x };
	t.tile.origin = this;

	xml_node<>* propNode = tile_node->first_node("properties");
	loadFromFile_TileProperties(propNode, t);

	tiles[get_ndx(t.tile.pos)] = t;
}

bool TilesetAsset::loadFromFile(const std::string& relpath) {

	bool texLoaded = false;

	assetFilePath = relpath;
	
	tiles.reset();
	tilesetRef.clear();
	tileLogic.clear();
	tileMat.clear();

	texTileSize = Vec2u{};

	bool r = true;
	std::unique_ptr<char[]> charPtr = readXML(assetFilePath + assetName + tilesetExt);

	if (charPtr) {
		char* xmlContent = charPtr.get();

		xml_document<>* doc = new xml_document<>();
		try {
			doc->parse<0>(xmlContent);

			xml_node<>* tilesetNode = doc->first_node("tileset");
			loadFromFile_Header(tilesetNode, relpath);

			tiles = std::make_unique<TileData[]>((size_t)texTileSize.x * texTileSize.y);

			// parse tiles
			xml_node<>* tileNode = tilesetNode->first_node("tile");
			while (tileNode) {

				loadFromFile_Tile(tileNode);
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

bool TilesetAsset::reloadFromFile() {
	bool loaded = false;
	try {
		TilesetAsset n_tile{getAssetName()};
		
		if (n_tile.loadFromFile(assetFilePath)) {
			*this = std::move(n_tile);

			for (size_t ndx = 0; ndx < texTileSize.x * texTileSize.y; ndx++) {
				tiles[ndx].tile.origin = this;
			}

			loaded = true;
		}
	}
	catch (std::exception)
	{
	}
	return loaded;
}

bool TilesetAsset::loadFromFlat(const flat::resources::TilesetAssetF* builder) 
{
	// TODO

	/*
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
	*/
	return false;
}

flatbuffers::Offset<flat::resources::TilesetAssetF> TilesetAsset::writeToFlat(flatbuffers::FlatBufferBuilder& builder) const 
{
	// TODO

	/*
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
	*/
	return false;
}

Tile TilesetAsset::getTile(Vec2u texPos) const {
	// assert this is actually on our texture
	assert(texPos.x < texTileSize.x && texPos.y < texTileSize.y);

	auto& r = tiles[get_ndx(texPos)];

	if (r.has_prop_bits & TileHasProp::HasTile) {
		return r.tile;
	}
	else {
		// create a default, empty tile here
		return Tile{
			.pos = texPos,
			.shape = TileShape(TileShape::Type::EMPTY, false, false),
			.origin = this,
		};
	}
}

TilesetAsset::TileLogicData TilesetAsset::getTileLogic(Vec2u position) const {
	// assert this is actually on our texture
	assert(position.x < texTileSize.x&& position.y < texTileSize.y);

	const auto& r = tiles[get_ndx(position)];

	TileLogicData data;

	if (r.has_prop_bits & TileHasProp::HasLogic) {
		data.logic_type = tileLogic.at(r.tileLogicNdx).logicType;
		if (r.has_prop_bits & TileHasProp::HasLogicArgs) {
			data.logic_args = tileLogic.at(r.tileLogicNdx).logicArg.at(r.tileLogicParamNdx);
		}
	}
	return data;
}

const TileMaterial& TilesetAsset::getMaterial(Vec2u position) const {
	// assert this is actually on our texture
	assert(position.x < texTileSize.x&& position.y < texTileSize.y);

	auto& r = tiles[get_ndx(position)];

	if (r.has_prop_bits & TileHasProp::HasMaterial) 
	{
		return Tile::getMaterial(tileMat.at(r.tileMatNdx));
	}
	else {
		return Tile::standardMat;
	}
}

void TilesetAsset::ImGui_getContent() {
	TextureAsset::ImGui_getContent();
}

}