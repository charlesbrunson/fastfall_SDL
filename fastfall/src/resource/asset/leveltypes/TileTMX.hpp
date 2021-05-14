#pragma once

#include "fastfall/resource/asset/LevelAssetTypes.hpp"

#include "fastfall/util/xml.hpp"

namespace ff {

namespace TileTMX {
	LayerRef parse(xml_node<>* layerNode, const TilesetMap& tilesets);
}

}
