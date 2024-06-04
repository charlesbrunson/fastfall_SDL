#pragma once


#include "fastfall/resource/asset/LevelAssetTypes.hpp"

#include "fastfall/util/xml.hpp"

namespace ff {

namespace TilesetTMX {
	TilesetMap parse(xml_node<>* tilesetNode);
}

}