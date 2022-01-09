#include "fastfall/resource/asset/TilesetAsset.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/game/level/TileLogic.hpp"

#include "fastfall/util/xml.hpp"
#include "fastfall/util/log.hpp"

#include "nlohmann/json.hpp"

#include <assert.h>
#include <fstream>

namespace ff {

const std::map<std::string, void(*)(TilesetAsset&, TilesetAsset::TileData&, char*)> TilesetAsset::tileProperties
{
	{"shape", [](TilesetAsset& asset, TileData& state, char* value)
	{
		state.tile.shape = TileShape::from_string(value);
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
			LOG_ERR_("Tileset: {}, unknown logic type for args at {}", asset.getAssetName(), state.tile.id.to_vec().to_string());
		}
	}},
	{"next_x", [](TilesetAsset& asset, TileData& state, char* value)
	{
		int v = std::stoi(value);
		if (v < 0) {
			state.tile.next_offset.x = 16u + v;
		}
		else {
			state.tile.next_offset.x = v;
		}
	}},
	{"next_y", [](TilesetAsset& asset, TileData& state, char* value)
	{
		int v = std::stoi(value);
		if (v < 0) {
			state.tile.next_offset.y = 16u + v;
		}
		else {
			state.tile.next_offset.y = v;
		}

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
			state.tile.matFacing = Cardinal::N;
		}
		else if (strcmp(value, "east")) {
			state.tile.matFacing = Cardinal::E;
		}
		else if (strcmp(value, "south")) {
			state.tile.matFacing = Cardinal::S;
		}
		else if (strcmp(value, "west")) {
			state.tile.matFacing = Cardinal::W;
		}
	}},

	// auto tile rules
	{"autotile", [](TilesetAsset& asset, TileData& state, char* value) 
	{
		using namespace nlohmann;

		json autotile_json;
		try {
			autotile_json = json::parse(value);
		}
		catch (json::exception except) {
			LOG_ERR_("json string parse failure: {}", except.what());
			LOG_ERR_("for tile: {}", Vec2u{ state.tile.id }.to_string());
			LOG_ERR_("json: {}", value);
			return;
		}

		static constexpr std::string_view card_dir[] = {
			"n", "e", "s", "w"
		};
		static constexpr std::string_view ord_dir[] = { 
			"nw", "ne", "se", "sw"
		};

		/*
		auto parse_rule = [](std::string_view value) -> AutoTileRule
		{
			if (value == "no") {
				return AutoTileRule{ .type = AutoTileRule::Type::No };
			}
			else if (value == "yes") {
				return AutoTileRule{ .type = AutoTileRule::Type::Yes };
			}
			else if (value == "n/a") {
				return AutoTileRule{ .type = AutoTileRule::Type::N_A };
			}
			else {
				return AutoTileRule{
					.type = AutoTileRule::Type::Yes,
					.shape = TileShape{ value.data() }
				};
			}
		};

		auto set_rule = [&](std::string_view key, std::string_view val) 
		{
			for (int i = 0; i < 4; i++)	{
				if (key == card_dir[i]) {
					state.tile.autotile_card[i] = parse_rule(val);
					break;
				}
			}
			for (int i = 0; i < 4; i++)	{
				if (key == ord_dir[i]) {
					state.tile.autotile_ord[i] = parse_rule(val);
					break;
				}
			}
		};
		*/

		for (auto it = autotile_json.begin(); it != autotile_json.end(); it++)
		{
			//set_rule(it.key(), it->get<std::string_view>());
		}
		return;
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

unsigned TilesetAsset::getTilesetRefIndex(std::string_view tileset_name) {
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

		char* value;
		if (propNode->first_attribute("value"))
		{
			value = propNode->first_attribute("value")->value();
		}
		else {
			value = propNode->value();
		}


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

	if (texTileSize.x > 16u || texTileSize.y > 16u)
		throw parse_error("tileset size must be 16x16 tiles or less", nullptr);

}

void TilesetAsset::loadFromFile_Tile(xml_node<>* tile_node)
{
	int id = atoi(tile_node->first_attribute("id")->value());

	TileData& t = tiles[id];
	t.has_prop_bits |= TileHasProp::HasTile;
	t.tile.id = TileID{ id % texTileSize.x, id / texTileSize.x };
	t.tile.origin = this;

	xml_node<>* propNode = tile_node->first_node("properties");
	loadFromFile_TileProperties(propNode, t);

	tiles[get_ndx(t.tile.id.to_vec())] = t;
}

bool TilesetAsset::loadFromFile(const std::string& relpath) {

	bool texLoaded = false;

	assetFilePath = relpath;
	
	tiles.clear();
	tilesetRef.clear();
	tileLogic.clear();
	tileMat.clear();

	texTileSize = Vec2u{};

	bool r = true;
	std::unique_ptr<char[]> charPtr = readXML(assetFilePath + assetName + tilesetExt);

	if (charPtr) {
		char* xmlContent = charPtr.get();

		auto doc = std::make_unique<xml_document<>>();

		try {
			doc->parse<0>(xmlContent);

			xml_node<>* tilesetNode = doc->first_node("tileset");
			loadFromFile_Header(tilesetNode, relpath);

			tiles = grid_vector<TileData>(texTileSize);

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

	assetName = builder->name()->c_str();
	texTileSize = *builder->tile_size();

	tilesetRef.clear();	
	tileLogic.clear();
	tileMat.clear();

	//tiles = std::make_unique<TileData[]>((size_t)texTileSize.x * texTileSize.y);
	tiles = grid_vector<TileData>(texTileSize);

	// load tilesets
	for (auto tileset : *builder->tilesets()) {
		tilesetRef.push_back(tileset->str());
	}

	// load materials
	for (auto material : *builder->materials()) {
		tileMat.push_back(material->str());
	}

	// load logics
	for (auto logic : *builder->logics()) {
		tileLogic.push_back(TilesetLogic{
			.logicType = logic->logic()->str()
			});
		for (auto arg : *logic->logic_arg()) {
			tileLogic.back().logicArg.push_back(arg->str());
		}
	}

	// load tile data
	size_t ndx = 0;
	for (auto tile_data : *builder->tile_data()) {
		TileData& t = tiles[ndx++];

		// shape
		t.tile.shape.type			= static_cast<TileShape::Type>(tile_data->tile().shape().type());
		//t.tile.shape.shapeTouches	= tile_data->tile().shape().shape_touches();
		t.tile.shape.flip_h		= tile_data->tile().shape().hflip();
		t.tile.shape.flip_v		= tile_data->tile().shape().vflip();

		// tile
		t.tile.id			= tile_data->tile().pos();
		t.tile.matFacing	= static_cast<Cardinal>(tile_data->tile().facing());
		t.tile.next_offset	= tile_data->tile().next_offset();
		t.tile.next_tileset = tile_data->tile().next_tileset_ndx();
		t.tile.origin = this;

		// tile data
		t.has_prop_bits		= tile_data->has_prop_bits();
		t.tileLogicNdx		= tile_data->logic_ndx();
		t.tileLogicParamNdx = tile_data->logic_arg_ndx();
		t.tileMatNdx		= tile_data->material_ndx();
	}

	loaded = tex.loadFromStream(builder->image()->Data(), builder->image()->size());
	return loaded;
}

flatbuffers::Offset<flat::resources::TilesetAssetF> TilesetAsset::writeToFlat(flatbuffers::FlatBufferBuilder& builder) const 
{
	using namespace flat::resources;
	using namespace flat::math;

	// write name
	auto flat_assetName = builder.CreateString(assetName);

	// write tile size
	Vec2Fu flat_tileSize{ texTileSize.x, texTileSize.y };

	// write tiles
	std::vector<TileDataF> tiledata_vec;

	for (const auto& tile_data : tiles)
	{
		TileShapeF flat_shape{
			static_cast<uint32_t>(tile_data.tile.shape.type),
			tile_data.tile.shape.flip_h,
			tile_data.tile.shape.flip_v
		};

		TileF flat_tile{
			tile_data.tile.id,
			flat_shape,
			static_cast<CardinalF>(tile_data.tile.matFacing),
			tile_data.tile.next_offset,
			tile_data.tile.has_next_tileset(),
			(tile_data.tile.next_tileset ? *tile_data.tile.next_tileset : 0u)
		};

		TileDataF tiledata{
			flat_tile,
			tile_data.has_prop_bits,
			tile_data.tileLogicNdx,
			tile_data.tileLogicParamNdx,
			tile_data.tileMatNdx
		};
		tiledata_vec.push_back(tiledata);
	}

	auto flat_tiledata = builder.CreateVectorOfStructs(tiledata_vec);

	// write tilesets
	auto flat_tilesets	= builder.CreateVectorOfStrings(tilesetRef);

	// write materials
	auto flat_materials = builder.CreateVectorOfStrings(tileMat);

	// write logic
	std::vector<flatbuffers::Offset<TilesetLogicF>> logic_vec;
	for (auto& logic : tileLogic)
	{
		auto flat_type = builder.CreateString(logic.logicType);
		auto flat_args = builder.CreateVectorOfStrings(logic.logicArg);

		TilesetLogicFBuilder flat_logic_builder(builder);
		flat_logic_builder.add_logic(flat_type);
		flat_logic_builder.add_logic_arg(flat_args);
		logic_vec.push_back(flat_logic_builder.Finish());
	}
	auto flat_logic = builder.CreateVector(logic_vec);

	// write image data
	assert(!fullpath.empty());
	std::vector<int8_t> texData = readFile(fullpath.c_str());
	auto flat_texdata = builder.CreateVector(texData);

	// finish
	TilesetAssetFBuilder tileBuilder(builder);
	tileBuilder.add_name(flat_assetName);
	tileBuilder.add_image(flat_texdata);
	tileBuilder.add_tile_size(&flat_tileSize);
	tileBuilder.add_tile_data(flat_tiledata);
	tileBuilder.add_tilesets(flat_tilesets);
	tileBuilder.add_materials(flat_materials);
	tileBuilder.add_logics(flat_logic);
	return tileBuilder.Finish();

}

Tile TilesetAsset::getTile(TileID texPos) const {
	// assert this is actually on our texture
	assert(texPos.x < texTileSize.x && texPos.y < texTileSize.y);

	auto& r = tiles[get_ndx(texPos)];

	if (r.has_prop_bits & TileHasProp::HasTile) {
		return r.tile;
	}
	else {
		// create a default, empty tile here
		return Tile{
			.id = texPos,
			.origin = this,
			.shape = {},
		};
	}
}

TilesetAsset::TileLogicData TilesetAsset::getTileLogic(TileID position) const {
	// assert this is actually on our texture
	assert(position.x < texTileSize.x && position.y < texTileSize.y);

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

const TileMaterial& TilesetAsset::getMaterial(TileID position) const {
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