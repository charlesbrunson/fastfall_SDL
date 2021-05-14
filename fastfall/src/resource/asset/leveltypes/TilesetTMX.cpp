#include "TilesetTMX.hpp"

#include "fastfall/resource/Asset.hpp"

namespace ff {

TilesetMap TilesetTMX::parse(xml_node<>* tilesetNode) {

	TilesetMap map;

	while (tilesetNode) {
		xml_attribute<>* tilesetAttr = tilesetNode->first_attribute();
		std::string ref;
		int fgid = 0;

		while (tilesetAttr) {
			char* attrName = tilesetAttr->name();
			char* attrVal = tilesetAttr->value();

			if (strcmp("firstgid", attrName) == 0) {
				fgid = atoi(attrVal);
				assert(fgid > 0);
			}
			else if (strcmp("source", attrName) == 0) {
				std::string source(attrVal);
				int relpathndx = source.find_last_of('/') + 1;
				source = source.substr(relpathndx, source.size() - relpathndx - strlen(tilesetExt));
				ref = std::move(source);
				assert(!ref.empty());
			}

			tilesetAttr = tilesetAttr->next_attribute();
		}
		map.insert(std::make_pair(fgid, std::move(ref)));
		tilesetNode = tilesetNode->next_sibling("tileset");
	}
	return map;
}

}