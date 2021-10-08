#pragma once


#include "fastfall/resource/asset/LevelAssetTypes.hpp"

#include "fastfall/util/xml.hpp"

namespace ff {

namespace ObjectTMX {
	LayerData parse(xml_node<>* layerNode);
}

}
