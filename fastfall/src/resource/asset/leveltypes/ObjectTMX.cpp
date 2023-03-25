#include "ObjectTMX.hpp"

#include "fastfall/util/math.hpp"
#include "fastfall/util/log.hpp"

namespace ff {

std::unordered_map<std::string, std::string> parseProperties(xml_node<>* propGroupNode) {
	std::unordered_map<std::string, std::string> properties;

	if (propGroupNode) {
		xml_node<>* propNode = propGroupNode->first_node("property");
		while (propNode) {
			char* name = propNode->first_attribute("name")->value();
			char* value = propNode->first_attribute("value")->value();

			if (name && value) {
				properties.insert(std::make_pair(name, value));
			}

			propNode = propNode->next_sibling();
		}
	}

	return properties;
}

std::vector<Vec2i> parsePoints(xml_node<>* polylineNode) {
	std::vector<Vec2i> points;

	if (polylineNode) {
		auto* polylinePoints = polylineNode->first_attribute("points");
		if (polylinePoints && polylinePoints->value()) {
			std::string_view view{ polylinePoints->value() };

			size_t ndxBegin = 0u;

			while (ndxBegin != std::string_view::npos) {
				size_t ndxEnd = view.find_first_of(' ', ndxBegin);

				std::string_view subview;

				if (ndxEnd != std::string_view::npos) {
					subview = view.substr(ndxBegin, ndxEnd - ndxBegin);
				}
				else {
					subview = view.substr(ndxBegin, view.length() - ndxBegin);
				}

				auto pivot = subview.find_first_of(',');
				auto xview = subview.substr(0u, pivot);
				auto yview = subview.substr(pivot + 1, subview.length() - pivot);

				Vec2i point;
				point.x = atoi(xview.data());
				point.y = atoi(yview.data());
				points.push_back(point);

				//LOG_INFO(point.to_string());

				if (ndxEnd != std::string_view::npos) {
					ndxBegin = ndxEnd + 1;
				}
				else {
					ndxBegin = std::string_view::npos;
				}
			}
		}
	}

	return points;
}

void parseObjectRefs(xml_node<>* objectNode, ObjectLayerData& objLayer) {

	while (objectNode) {
		LevelObjectData objdata;

		objdata.level_id.id = atoi(objectNode->first_attribute("id")->value());

		auto* type = objectNode->first_attribute("class");
		if (type && type->value()) {
			objdata.typehash = std::hash<std::string_view>{}(type->value());
		}
		else {
			objdata.typehash = 0u;
		}

		auto* name = objectNode->first_attribute("name");
		if (name && name->value()) {
			objdata.name = name->value();
		}
		else {
			objdata.name = "";
		}

		Rectu area;
		area.left = atoi(objectNode->first_attribute("x")->value());
		area.top = atoi(objectNode->first_attribute("y")->value());

		auto* attr = objectNode->first_attribute("width");
		area.width = attr ? atoi(attr->value()) : 0;
		attr = objectNode->first_attribute("height");
		area.height = attr ? atoi(attr->value()) : 0;

		objdata.area = area;

		//objdata.position.x = area.left + area.width / 2;
		//objdata.position.y = area.top + area.height;

		objdata.properties = parseProperties(objectNode->first_node("properties"));
		objdata.points = parsePoints(objectNode->first_node("polyline"));

		objLayer.objects.push_back(std::move(objdata));

		objectNode = objectNode->next_sibling();
	}
}


/////////////////////////////////////////////////////////////

ObjectLayerData ObjectTMX::parse(xml_node<>* layerNode) {
	ObjectLayerData layer;
	layer.layer_id = atoi(layerNode->first_attribute("id")->value());

	if (auto* name_attr = layerNode->first_attribute("name")) {
		layer.layer_name = name_attr->value();
	}
	else {
		layer.layer_name = "unknown";
	}
	parseObjectRefs(layerNode->first_node("object"), layer);
	return layer;
}

}