#include "fastfall/resource/asset/TilesetAsset.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/util/xml.hpp"
#include "fastfall/util/log.hpp"

#include "nlohmann/json.hpp"

#include "fastfall/user_types.hpp"

#include <assert.h>
#include <fstream>

namespace ff {

const std::map<std::string, void(*)(TilesetAsset&, TilesetAsset::TileData&, char*)> TilesetAsset::tileProperties
{
	{"shape", [](TilesetAsset& asset, TileData& state, char* value)
	{
		state.tile.shape = TileShape::from_string(value);
		state.has_prop_bits |= TileHasProp::HasTile;

		if (state.has_prop_bits & TileHasProp::HasConstraint)
		{
			asset.constraints.at(state.tileConstraint).shape = state.tile.shape;
		}
	}},

	// logic stuff
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
			LOG_ERR_("Tileset: {}, unknown logic type for args at {}", asset.asset_path.generic_string(), state.tile.id.to_vec().to_string());
		}
	}},
	{"next_x", [](TilesetAsset& asset, TileData& state, char* value)
	{
		int v = std::stoi(value);
		auto& next = state.tile.next_offset;
		next = TileID{
            (v < 0		  ? TileID::dimension_max + v : v),
            (next.valid() ? next.getY()				  : 0u)
		};
	}},
	{"next_y", [](TilesetAsset& asset, TileData& state, char* value)
	{
		int v = std::stoi(value);

		auto& next = state.tile.next_offset;
		next = TileID{ 
			next.valid()	? next.getX()				: 0u, 
			v < 0			? TileID::dimension_max + v : v 
		};
	}},
	{"next_tileset", [](TilesetAsset& asset, TileData& state, char* value)
	{
        auto str = asset.asset_path.generic_string();
		if (strlen(value) > 0 && strcmp(str.c_str(), value) != 0) {
			state.tile.next_tileset = asset.getTilesetRefIndex(value);
		}
	}},

	// material
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

	// auto tile constraints
	{"auto_constraint", [](TilesetAsset& asset, TileData& state, char* value) 
	{
		using namespace nlohmann;

		if (state.tile.auto_substitute)
		{
			LOG_ERR_("cannot set auto_constraint for tile with auto_substitute set: {}", state.tile.id.to_vec());
			return;
		}

		json autotile_json;
		try {
			autotile_json = json::parse(value);
		}
		catch (json::exception except) {
			LOG_ERR_("json string parse failure: {}", except.what());
			LOG_ERR_("for tile: {}", state.tile.id.to_vec().to_string());
			LOG_ERR_("json: {}", value);
			return;
		}

		TileConstraint constraint{
			.tile_id = state.tile.id,
			.shape = state.tile.shape
		};

		static constexpr cardinal_array<std::string_view> card_str = {
			"n", "e", "s", "w"
		};
		static constexpr ordinal_array<std::string_view> ord_str = {
			"nw", "ne", "se", "sw"
		};
		
		auto set_rule = [&](std::string_view key, std::string_view val) 
		{
			for (auto dir : direction::cardinals) {
				if (key == card_str[dir]) {
					if (val == "no") {
						constraint.edges[dir] = tile_constraint_options::no;
					}
					else if (val == "yes") {
						constraint.edges[dir] = tile_constraint_options::yes;
					}
					else if (val == "n/a") {
						constraint.edges[dir] = tile_constraint_options::n_a;
					}
					else {
						constraint.edges[dir] = TileShape::from_string(val);
					}
					break;
				}
			}
			for (auto dir : direction::ordinals) {
				if (key == ord_str[dir]) {
					if (val == "no") {
						constraint.corners[dir] = tile_constraint_options::no;
					}
					else if (val == "yes") {
						constraint.corners[dir] = tile_constraint_options::yes;
					}
					else if (val == "n/a") {
						constraint.corners[dir] = tile_constraint_options::n_a;
					}
					break;
				}
			}
		};
		
		for (auto it = autotile_json.begin(); it != autotile_json.end(); it++)
		{
			set_rule(it.key(), it->get<std::string_view>());
		}

		state.has_prop_bits |= TileHasProp::HasConstraint;
		state.tileConstraint = asset.addTileConstraint(state.tile.id, constraint);
	}},

	// make tile as auto substitutable
	{ "auto_substitute", [](TilesetAsset& asset, TileData& state, char* value)
	{
		bool has = std::string_view{value} == "true";
		if (has && ((state.has_prop_bits & TileHasProp::HasConstraint) > 0))
		{
			LOG_ERR_("cannot set auto_substitute for tile with auto_constraint set: {}", state.tile.id.to_vec());
		}
		else {
			state.tile.auto_substitute = has;
		}
	}},

	//padding
	{ "padding", [](TilesetAsset& asset, TileData& state, char* value)
	{
		std::string_view str{value};
		state.tile.id.setPadding(Cardinal::N, str.find_first_of('n') != std::string_view::npos);
		state.tile.id.setPadding(Cardinal::E, str.find_first_of('e') != std::string_view::npos);
		state.tile.id.setPadding(Cardinal::S, str.find_first_of('s') != std::string_view::npos);
		state.tile.id.setPadding(Cardinal::W, str.find_first_of('w') != std::string_view::npos);

		if (state.has_prop_bits & TileHasProp::HasConstraint)
		{
			asset.constraints.at(state.tileConstraint).tile_id = state.tile.id;
		}

	}},
    { "framecount", [](TilesetAsset& asset, TileData& state, char* value) {
        state.frameCount = (std::max)(uint8_t{ 1 }, static_cast<uint8_t>(std::stoi(value)));
    }},
    { "framedelay", [](TilesetAsset& asset, TileData& state, char* value) {
        state.frameDelay = (std::max)(uint8_t{ 1 }, static_cast<uint8_t>(std::stoi(value)));
    }},
};

TilesetAsset::TilesetAsset(const std::filesystem::path& t_asset_path) :
	TextureAsset(t_asset_path)
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


unsigned TilesetAsset::addTileConstraint(TileID tile_id, TileConstraint constraint)
{
	constraints.push_back(constraint);
	return constraints.size() - 1;
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


void TilesetAsset::loadFromFile_Header(xml_node<>* tileset_node)
{
	int columns = atoi(tileset_node->first_attribute("columns")->value());
	if (columns == 0)
		throw parse_error("columns == 0", nullptr);

	xml_node<>* imgNode = tileset_node->first_node("image");
	if (!imgNode)
		throw parse_error("no image node", nullptr);

	std::string source = imgNode->first_attribute("source")->value();
    set_texture_path(asset_path.parent_path() / source);
	if (!TextureAsset::loadFromFile())
		throw parse_error("could not load sprite source", nullptr);

	texTileSize = Vec2u(tex.size()) / TILESIZE;

	if (texTileSize.x > TileID::dimension_max 
		|| texTileSize.y > TileID::dimension_max)
		throw parse_error("tileset size must be 64x64 tiles or less", nullptr);

}

void TilesetAsset::loadFromFile_Tile(xml_node<>* tile_node)
{
	int id = atoi(tile_node->first_attribute("id")->value());

	TileData& t = tiles[id];
	//t.has_prop_bits |= TileHasProp::HasTile;
	t.tile.id = TileID{ id % texTileSize.x, id / texTileSize.x };
	t.tile.origin = this;

	xml_node<>* propNode = tile_node->first_node("properties");
	loadFromFile_TileProperties(propNode, t);

	if (t.has_prop_bits & TileHasProp::HasTile) {
		tiles[t.tile.id.to_vec()] = t;
	}
}

bool TilesetAsset::loadFromFile() {

	bool texLoaded = false;

	tiles.clear();
	tilesetRef.clear();
	tileLogic.clear();
	tileMat.clear();
	constraints.clear();
	auto_shape_cache.clear();

	texTileSize = Vec2u{};

	bool r = true;
	std::unique_ptr<char[]> charPtr = readXML(asset_path);

	if (charPtr) {
		char* xmlContent = charPtr.get();

		auto doc = std::make_unique<xml_document<>>();

		try {
			doc->parse<0>(xmlContent);

			xml_node<>* tilesetNode = doc->first_node("tileset");
			loadFromFile_Header(tilesetNode);

			tiles = grid_vector<TileData>(texTileSize);

			// parse tiles
			xml_node<>* tileNode = tilesetNode->first_node("tile");
			while (tileNode) {

				loadFromFile_Tile(tileNode);
				tileNode = tileNode->next_sibling();
			}
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
	return r;
}

bool TilesetAsset::reloadFromFile() {
	bool loaded = false;
	try {
		TilesetAsset n_tile{ asset_path };
		
		if (n_tile.loadFromFile()) {
			*this = std::move(n_tile);
			for (auto& tile_data : tiles)
			{
				tile_data.tile.origin = this;
			}
			loaded = true;
		}
	}
	catch (std::exception)
	{
	}
	return loaded;
}

std::optional<Tile> TilesetAsset::getTile(TileID tile_id) const {
	// assert this is actually on our texture
	assert(tile_id.getX() < texTileSize.x && tile_id.getY() < texTileSize.y);

	auto& r = tiles[tile_id.to_vec()];
	if (r.has_prop_bits & TileHasProp::HasTile) {
		return r.tile;
	}
	else {
		return {};
	}
}

TilesetAsset::TileLogicData TilesetAsset::getTileLogic(TileID tile_id) const {
	// assert this is actually on our texture
	assert(tile_id.getX() < texTileSize.x && tile_id.getY() < texTileSize.y);

	const auto& r = tiles[tile_id.to_vec()];

	TileLogicData data;

	if (r.has_prop_bits & TileHasProp::HasLogic) {
		data.logic_type = tileLogic.at(r.tileLogicNdx).logicType;
		if (r.has_prop_bits & TileHasProp::HasLogicArgs) {
			data.logic_args = tileLogic.at(r.tileLogicNdx).logicArg.at(r.tileLogicParamNdx);
		}
	}
	return data;
}

const TileMaterial& TilesetAsset::getMaterial(TileID tile_id) const {
	// assert this is actually on our texture
	assert(tile_id.getX() < texTileSize.x && tile_id.getY() < texTileSize.y);

	auto& r = tiles[tile_id.to_vec()];

	if (r.has_prop_bits & TileHasProp::HasMaterial)
	{
		return ff::user_types::get_tile_material(tileMat.at(r.tileMatNdx));
	}
	else {
		return TileMaterial::standard;
	}
}


std::optional<TileID> TilesetAsset::getAutoTileForShape(TileShape shape) const
{
	for (auto& [cached_shape, cached_id] : auto_shape_cache)
	{
		if (cached_shape == shape)
		{
			return cached_id;
		}
	}
	for (auto& tile : tiles)
	{
		if (tile.tile.auto_substitute && tile.tile.shape == shape)
		{
			auto_shape_cache.push_back({ tile.tile.shape, tile.tile.id });
			return tile.tile.id;
		}
	}
	return {};
}

uint8_t TilesetAsset::getFrameCount(TileID tile_id) const {
    assert(tile_id.getX() < texTileSize.x && tile_id.getY() < texTileSize.y);
    auto& r = tiles[tile_id.to_vec()];
    return r.frameCount;
}

uint8_t TilesetAsset::getFrameDelay(TileID tile_id) const {
    assert(tile_id.getX() < texTileSize.x && tile_id.getY() < texTileSize.y);
    auto& r = tiles[tile_id.to_vec()];
    return r.frameDelay;
}

void TilesetAsset::ImGui_getContent() {
	TextureAsset::ImGui_getContent();
}

}