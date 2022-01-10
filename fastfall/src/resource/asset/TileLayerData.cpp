#include "fastfall/resource/asset/TileLayerData.hpp"

#include <assert.h>

#include "fastfall/util/base64.hpp"
#include "fastfall/resource/Resources.hpp"

namespace ff {


TileLayerData::TileLayerData()
	: layer_id(0)
	, tileSize(Vec2u{ 0, 0 })
{
}

TileLayerData::TileLayerData(unsigned id)
	: layer_id(id)
	, tileSize(Vec2u{0, 0})
{
}

TileLayerData::TileLayerData(unsigned id, Vec2u size)
	: layer_id(id)
	, tileSize(size)
	, tiles(size)
	, shapes(size)
{
}


void TileLayerData::resize(Vec2u size, Vec2i offset) {
	TileLayerData n_data(layer_id, size);

	Recti old_area{ { 0, 0 }, Vec2i{ tileSize } };
	Recti new_area{ offset, Vec2i{ size } };

	Recti intersection;

	if (old_area.intersects(new_area, intersection))
	{
		for (auto& tile : tiles.take_view(intersection.getPosition(), intersection.getSize()))
		{
			if (!tile.has_tile)
				continue;

			n_data.setTile(tile.pos + offset, tile.tile_id, tilesets[tile.tileset_ndx].name);
		}
	}

	*this = std::move(n_data);
}

void TileLayerData::setParallax(bool enabled, Vec2u parallax_size)
{
	if ((enabled && !has_collision) || !enabled)
	{
		has_parallax = enabled;
		parallaxSize = parallax_size;
	}
	else {
		LOG_ERR_("unable to enable parallax while collision is enabled");
	}
}

void TileLayerData::setScroll(bool enabled, Vec2f scroll_rate)
{
	if ((enabled && !has_collision) || !enabled)
	{
		has_scroll = enabled;
		scrollrate = scroll_rate;
	}
	else {
		LOG_ERR_("unable to enable parallax while collision is enabled");
	}
}

void TileLayerData::setCollision(bool enabled, unsigned border)
{
	if ((enabled && !has_scroll && !has_parallax) || !enabled)
	{
		has_collision = enabled;
		collision_border_bits = border;
	}
	else {
		LOG_ERR_("unable to enable collision while scrolling or parallax is enabled");
	}
}

TileID TileLayerData::setTile(Vec2u at, TileID tile_id, const TilesetAsset& tileset) {
	assert(at.x < tileSize.x && at.y < tileSize.y);

	auto it = std::find_if(tilesets.begin(), tilesets.end(), 
		[&tileset](const auto& tileset_data) { return tileset.getAssetName() == tileset_data.name; }
	);
	uint8_t tileset_ndx = UINT8_MAX;
	if (it != tilesets.end()) {

		uint8_t p_ndx = tiles[at].tileset_ndx;
		tileset_ndx = std::distance(tilesets.begin(), it);

		if (p_ndx != UINT8_MAX)	{
			// replacing a tile
			tilesets[p_ndx].tile_count--;
		}

		it->tile_count++;

		if (tileset_ndx == 1)
			LOG_INFO("{}", it->tile_count);
	}
	else if (tilesets.size() <= UINT8_MAX) {
		tileset_ndx = tilesets.size();
		tilesets.push_back({ 
			.name = tileset.getAssetName(),
			.tile_count = 1 
			});
	}
	else {
		LOG_ERR_("unable to set tile, tileset max reached for layer: {}", tilesets.size());
	}

	Tile tile = tileset.getTile(tile_id);
	TileID placed_tiled_id = tile_id;

	if (tile.auto_substitute)
	{
		auto state = get_autotile_state(tile.shape, shapes, at);
		auto opt_tile_id = auto_best_tile(state, tileset.getConstraints());
		placed_tiled_id = opt_tile_id.value_or(placed_tiled_id);
	}

	tiles[at].has_tile = true;
	tiles[at].pos = at;
	tiles[at].tile_id = placed_tiled_id;
	tiles[at].tileset_ndx = tileset_ndx;
	tiles[at].is_autotile = tile.auto_substitute;
	shapes[at] = tile.shape;
	return tiles[at].tile_id;
}

std::pair<bool, unsigned> TileLayerData::removeTile(Vec2u at)
{
	assert(at.x < tileSize.x&& at.y < tileSize.y);
	//size_t i = at.x + (at.y * tileSize.x);

	bool erased = false;
	unsigned count = 0;

	if (TileData& tile = tiles[at]; tile.has_tile)
	{
		erased = true;

		count = --tilesets[tile.tileset_ndx].tile_count;
		if (tilesets[tile.tileset_ndx].tile_count == 0)
		{
			tilesets.erase(tilesets.begin() + tile.tileset_ndx);
			std::for_each(tiles.begin(), tiles.end(),
				[&](TileData& tt) {
					if (tt.tileset_ndx > tile.tileset_ndx)
						tt.tileset_ndx--;
				});
		}

		tile = TileData{};
		shapes[at] = TileShape{};
	}
	return { erased, count };
}

void TileLayerData::clearTiles() {
	tilesets.clear();
	tiles = grid_vector<TileData>(tileSize);
	shapes = grid_vector<TileShape>(tileSize);
}

// SERIALIZATION

TileLayerData TileLayerData::loadFromTMX(xml_node<>* layerNode, const TilesetMap& tilesets)
{
	unsigned id = atoi(layerNode->first_attribute("id")->value());
	unsigned size_x = atoi(layerNode->first_attribute("width")->value());
	unsigned size_y = atoi(layerNode->first_attribute("height")->value());

	TileLayerData layer(id, Vec2u{ size_x, size_y });

	// PARSE LAYER PROPERTIES

	struct TileLayerProps {
		bool has_parallax = false;
		bool has_scroll = false;
		bool has_collision = false;
		Vec2u parallaxSize;
		Vec2f scrollrate;
		unsigned collision_border_bits = 0u;
	} properties;

	const static std::map<std::string, void(*)(TileLayerProps&, char*)> validLayerProps{
		// collision
		{"collision", [](TileLayerProps& layer, char* val) {
			layer.has_collision = strcmp(val, "true") == 0;
		}},
		{"collision_border", [](TileLayerProps& layer, char* val) {
			std::string str(val);
			layer.collision_border_bits = 0u;
			for (char c : str) {
				switch (c) {
				case 'N': layer.collision_border_bits |= direction::to_bits(Cardinal::N); break;
				case 'E': layer.collision_border_bits |= direction::to_bits(Cardinal::E);  break;
				case 'S': layer.collision_border_bits |= direction::to_bits(Cardinal::S); break;
				case 'W': layer.collision_border_bits |= direction::to_bits(Cardinal::W);  break;
				}
			}
		}},

		// parallax flag
		{"parallax", [](TileLayerProps& layer, char* val) {
			layer.has_parallax = strcmp(val, "true") == 0;
		}},
		{"parallax_sizex", [](TileLayerProps& layer, char* val) {
			layer.parallaxSize.x = atoi(val);
		}},
		{"parallax_sizey", [](TileLayerProps& layer, char* val) {
			layer.parallaxSize.y = atoi(val);
		}},

		// layer scrolling
		{"scroll", [](TileLayerProps& layer, char* val) {
			layer.has_scroll = strcmp(val, "true") == 0;
		}},
		{"scroll_velx", [](TileLayerProps& layer, char* val) {
			layer.scrollrate.x = atof(val);
		}},
		{"scroll_vely", [](TileLayerProps& layer, char* val) {
			layer.scrollrate.y = atof(val);
		}},
	};

	if (auto propNode = layerNode->first_node("properties")) {
		xml_node<>* prop = propNode->first_node("property");
		while (prop) {
			char* attrName = prop->first_attribute("name")->value();
			char* attrValue = prop->first_attribute("value")->value();

			auto it = validLayerProps.find(attrName);
			if (it != validLayerProps.end()) {
				it->second(properties, attrValue);
			}
			else {
				LOG_ERR_("unknown property: {}, {}", attrName, attrValue);
			}

			prop = prop->next_sibling();
		}
	}

	// APPLY PROPERTIES
	layer.setCollision(properties.has_collision, properties.collision_border_bits);
	layer.setParallax(properties.has_parallax, properties.parallaxSize);
	layer.setScroll(properties.has_scroll, properties.scrollrate);

	// PARSE TILES
	if (auto dataNode = layerNode->first_node("data"))
	{
		// flip flags set by TMX filetype
		static constexpr unsigned FLIPPED_FLAGS = 0xE000'0000;

		//assert this is encoded as base64
		assert(dataNode&& strcmp("base64", dataNode->first_attribute("encoding")->value()) == 0);

		// decode
		std::string dataStr(dataNode->value());
		dataStr.erase(std::remove_if(dataStr.begin(), dataStr.end(), ::isspace), dataStr.end());

		std::vector<uint8_t> data = base64_decode(dataStr);
		size_t dataSize = data.size();

		Vec2u tilepos(0u, 0u);
		for (size_t i = 0; i < dataSize; i += 4) {

			gid tilesetgid =
				data[i] |
				data[i + 1] << 8 |
				data[i + 2] << 16 |
				data[i + 3] << 24;

			// dont need these
			tilesetgid &= ~FLIPPED_FLAGS;

			if (tilesetgid > 0)
			{
				auto it = std::find_if(tilesets.rbegin(), tilesets.rend(), [tilesetgid](const auto& pair) {
					return tilesetgid >= pair.first;
					});
				
				TilesetAsset* tileAsset = Resources::get<TilesetAsset>(it->second);
				int columns = tileAsset->getTileSize().x;

				TileID texture_pos{ (tilesetgid - it->first) % columns, (tilesetgid - it->first) / columns };

				layer.setTile(tilepos, texture_pos, *tileAsset);
			}

			tilepos.x++;
			if (tilepos.x == layer.tileSize.x) {
				tilepos.x = 0;
				tilepos.y++;
			}
		}
	}
	return layer;
}

}