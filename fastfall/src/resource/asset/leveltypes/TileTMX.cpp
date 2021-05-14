#include "TileTMX.hpp"

#include "fastfall/util/base64.hpp"

#include "fastfall/resource/Resources.hpp"

#include <functional>

namespace ff {

void parseLayerProperties(xml_node<>* propNode, TileLayerRef& layer) {

	const static std::map<const std::string, std::function<void(TileLayerRef&, char*)>> validLayerProps
	{
		/*
		{"ACTIVE", [](LayerRef& layer, char* val) {
			layer.isActive = strcmp("true", val) == 0;
		}},
		*/
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

	// flags set by TMX filetype
	static constexpr unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
	static constexpr unsigned FLIPPED_VERTICALLY_FLAG = 0x40000000;
	static constexpr unsigned FLIPPED_DIAGONALLY_FLAG = 0x20000000;

	//assert this is encoded as base64
	assert(dataNode && strcmp("base64", dataNode->first_attribute("encoding")->value()) == 0);

	// decode
	std::string dataStr(dataNode->value());
	dataStr.erase(std::remove_if(dataStr.begin(), dataStr.end(), ::isspace), dataStr.end());

	std::vector<int8_t> data = base64_decode(dataStr);
	size_t dataSize = data.size();

	// read decoded tile data
	Vec2u tilepos(0u, 0u);
	for (unsigned i = 0; i < dataSize; i += 4) {
		gid tilesetgid =
			data[i] |
			data[i + 1] << 8 |
			data[i + 2] << 16 |
			data[i + 3] << 24;

		// dont need these
		tilesetgid &= ~(FLIPPED_HORIZONTALLY_FLAG |
			FLIPPED_VERTICALLY_FLAG |
			FLIPPED_DIAGONALLY_FLAG);

		if (tilesetgid > 0) {
			TileRef t;
			t.tile_id = tilesetgid;

			// calc tileset dependency and tex position
			gid fgid = tilesets.cbegin()->first;
			for (auto& tileset : tilesets) {

				if (tileset.first > tilesetgid) {
					t.tilesetName = &tilesets.at(fgid);
					break;
				}
				fgid = tileset.first;
			}
			if (!t.tilesetName)
				t.tilesetName = &tilesets.at(fgid);

			TilesetAsset* tileAsset = Resources::get<TilesetAsset>(*t.tilesetName);
			int columns = tileAsset->getTileSize().x;

			Vec2u texture_pos;
			texture_pos.x = ((tilesetgid - fgid) % columns);
			texture_pos.y = ((tilesetgid - fgid) / columns);
			t.texPos = texture_pos;

			layer.tiles.insert(std::make_pair(tilepos, t));
			//layer.collision.setTile(tilepos, tileAsset->getTile(t.texPos).shape);
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

	LayerRef layer(LayerType::TILELAYER);

	layer.id = atoi(layerNode->first_attribute("id")->value());
	layer.tileLayer->tileSize.x = atoi(layerNode->first_attribute("width")->value());
	layer.tileLayer->tileSize.y = atoi(layerNode->first_attribute("height")->value());

	parseLayerProperties(layerNode->first_node("properties"), *layer.tileLayer.get());

	parseLayerTiles(layerNode->first_node("data"), *layer.tileLayer.get(), tilesets);

	return layer;
}

}