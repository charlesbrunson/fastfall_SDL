#include "TileTMX.hpp"

#include "fastfall/util/base64.hpp"

#include "fastfall/resource/Resources.hpp"

#include <functional>

#include "fastfall/util/log.hpp"

namespace ff {

void parseLayerProperties(xml_node<>* propNode, TileLayerRef& layer) {
	const static std::map < std::string, void(*)(TileLayerRef&, char*) > validLayerProps{
		// collision
		{"collision", [](TileLayerRef& layer, char* val) {
			layer.has_collision = strcmp(val, "true") == 0;
		}},
		{"collision_border", [](TileLayerRef& layer, char* val) {
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
		{"parallax", [](TileLayerRef& layer, char* val) {
			layer.has_parallax = strcmp(val, "true") == 0;
		}},
		{"parallax_sizex", [](TileLayerRef& layer, char* val) {
			layer.parallaxSize.x = atoi(val);
		}},
		{"parallax_sizey", [](TileLayerRef& layer, char* val) {
			layer.parallaxSize.y = atoi(val);
		}},

		// layer scrolling
		{"scroll", [](TileLayerRef& layer, char* val) {
			layer.has_scroll = strcmp(val, "true") == 0;
		}},
		{"scroll_velx", [](TileLayerRef& layer, char* val) {
			layer.scrollrate.x = atof(val);
		}},
		{"scroll_vely", [](TileLayerRef& layer, char* val) {
			layer.scrollrate.y = atof(val);
		}},
	};

	if (propNode) {
		xml_node<>* prop = propNode->first_node("property");
		while (prop) {
			char* attrName = prop->first_attribute("name")->value();
			char* attrValue = prop->first_attribute("value")->value();

			auto it = validLayerProps.find(attrName);
			if (it != validLayerProps.end()) {
				it->second(layer, attrValue);
			}

			prop = prop->next_sibling();
		}
	}
}

void parseLayerTiles(xml_node<>* dataNode, TileLayerRef& layer, const TilesetMap& tilesets) {

	// flip flags set by TMX filetype
	static constexpr unsigned FLIPPED_FLAGS = 0xE0000000;

	//assert this is encoded as base64
	assert(dataNode && strcmp("base64", dataNode->first_attribute("encoding")->value()) == 0);

	// decode
	std::string dataStr(dataNode->value());
	dataStr.erase(std::remove_if(dataStr.begin(), dataStr.end(), ::isspace), dataStr.end());

	std::vector<int8_t> data = base64_decode(dataStr);
	size_t dataSize = data.size();

	// read decoded tile data
	Vec2u tilepos(0u, 0u);
	for (size_t i = 0; i < dataSize; i += 4) {
		gid tilesetgid =
			data[i] |
			data[i + 1] << 8 |
			data[i + 2] << 16 |
			data[i + 3] << 24;

		// dont need these
		tilesetgid &= ~FLIPPED_FLAGS;

		if (tilesetgid > 0 
			&& (layer.parallaxSize.x == 0 || tilepos.x < layer.parallaxSize.x)
			&& (layer.parallaxSize.y == 0 || tilepos.y < layer.parallaxSize.y)) 
		{
			TileRef t;
			//t.tile_id = tilesetgid;

			// calc tileset dependency and tex position
			gid fgid = tilesets.cbegin()->first;
			for (auto& tileset : tilesets) {

				if (tileset.first > tilesetgid) {
					t.tilesetName = tilesets.at(fgid);
					break;
				}
				fgid = tileset.first;
			}
			if (!t.tilesetName.data())
				t.tilesetName = tilesets.at(fgid);

			TilesetAsset* tileAsset = Resources::get<TilesetAsset>(t.tilesetName);
			int columns = tileAsset->getTileSize().x;

			Vec2u texture_pos;
			texture_pos.x = ((tilesetgid - fgid) % columns);
			texture_pos.y = ((tilesetgid - fgid) / columns);
			t.tile_id = tilesetgid;
			t.texPos = texture_pos;
			t.tilePos = tilepos;

			layer.tiles.push_back(std::move(t));
		}

		tilepos.x++;
		if (tilepos.x == layer.tileSize.x) {
			tilepos.x = 0;
			tilepos.y++;
		}
	}
}

/////////////////////////////////////////////////////////////

LayerRef TileTMX::parse(xml_node<>* layerNode, const TilesetMap& tilesets) {

	LayerRef layer(LayerRef::Type::Tile);

	layer.id = atoi(layerNode->first_attribute("id")->value());

	TileLayerRef& tileLayer = std::get<TileLayerRef>(layer.layer);

	tileLayer.tileSize.x = atoi(layerNode->first_attribute("width")->value());
	tileLayer.tileSize.y = atoi(layerNode->first_attribute("height")->value());

	parseLayerProperties(layerNode->first_node("properties"), tileLayer);

	if (tileLayer.has_collision && (tileLayer.has_scroll || tileLayer.has_parallax)) {
		throw "compilation failure: cannot have a tile layer with collision and with scrolling or parallax";
	}
	if (tileLayer.has_parallax && tileLayer.parallaxSize == Vec2u{}) {
		throw "compilation failure: parallax set but size is not defined";
	}

	parseLayerTiles(layerNode->first_node("data"), tileLayer, tilesets);

	return layer;
}

}
