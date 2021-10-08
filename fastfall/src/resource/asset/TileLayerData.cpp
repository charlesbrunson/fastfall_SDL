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
{
	size_t count = size.x * size.y;
	tiles.has_tile.resize(count, false);
	tiles.pos.resize(count, Vec2u{});
	tiles.tex_pos.resize(count, Vec2u{});
	tiles.tileset_ndx.resize(count, UINT8_MAX);
}


void TileLayerData::resize(Vec2u size, Vec2i offset) {
	TileLayerData n_data(layer_id, size);

	for (int i = 0; i < size.x * size.y; i++)
	{
		Vec2i pos = Vec2i{ tiles.pos[i] } + offset;
		if (tiles.has_tile[i]
			&& pos.x >= 0 && pos.x < size.x
			&& pos.y >= 0 && pos.y < size.x)
		{
			n_data.setTile(Vec2u{ pos }, tiles.tex_pos[i], tilesets[tiles.tileset_ndx[i]].first);
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

void TileLayerData::setTile(Vec2u at, Vec2u tex, const std::string& tileset) {
	assert(at.x < tileSize.x&& at.y < tileSize.y);

	size_t i = at.x + (at.y * tileSize.x);


	auto it = std::find_if(tilesets.begin(), tilesets.end(), [&tileset](const auto& pair) { return tileset == pair.first; });

	if (it != tilesets.end()) {
		tiles.tileset_ndx[i] = std::distance(tilesets.begin(), it);
		it->second++;
	}
	else if (tilesets.size() <= UINT8_MAX) {
		tiles.tileset_ndx[i] = tilesets.size();
		tilesets.push_back(std::make_pair(tileset, 1));
	}
	else {
		LOG_ERR_("unable to set tile, tileset max reached for layer: {}", tilesets.size());
	}

	tiles.has_tile[i] = true;
	tiles.pos[i] = at;
	tiles.tex_pos[i] = tex;
}

void TileLayerData::removeTile(Vec2u at)
{
	assert(at.x < tileSize.x&& at.y < tileSize.y);
	size_t i = at.x + (at.y * tileSize.x);

	if (tiles.has_tile[i])
	{
		tilesets[tiles.tileset_ndx[i]].second--;
		if (tilesets[tiles.tileset_ndx[i]].second == 0)
		{
			tilesets.erase(tilesets.begin() + i);
			for_each(tiles.tileset_ndx.begin(), tiles.tileset_ndx.end(),
				[i](uint8_t& ndx) {
					if (ndx > i)
						ndx--;
				});
		}

		tiles.has_tile[i] = false;
		tiles.pos[i] = Vec2u{};
		tiles.tex_pos[i] = Vec2u{};
		tiles.tileset_ndx[i] = UINT8_MAX;
	}
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
				case 'N': layer.collision_border_bits |= cardinalToBits(Cardinal::NORTH); break;
				case 'E': layer.collision_border_bits |= cardinalToBits(Cardinal::EAST);  break;
				case 'S': layer.collision_border_bits |= cardinalToBits(Cardinal::SOUTH); break;
				case 'W': layer.collision_border_bits |= cardinalToBits(Cardinal::WEST);  break;
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
		static constexpr unsigned FLIPPED_FLAGS = 0xE0000000;

		//assert this is encoded as base64
		assert(dataNode&& strcmp("base64", dataNode->first_attribute("encoding")->value()) == 0);

		// decode
		std::string dataStr(dataNode->value());
		dataStr.erase(std::remove_if(dataStr.begin(), dataStr.end(), ::isspace), dataStr.end());

		std::vector<int8_t> data = base64_decode(dataStr);
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

				Vec2u texture_pos;
				texture_pos.x = ((tilesetgid - it->first) % columns);
				texture_pos.y = ((tilesetgid - it->first) / columns);

				layer.setTile(tilepos, texture_pos, tileAsset->getAssetName());
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