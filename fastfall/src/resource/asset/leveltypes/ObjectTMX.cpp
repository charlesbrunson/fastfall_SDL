#include "ObjectTMX.hpp"

#include "fastfall/util/math.hpp"
#include "fastfall/util/log.hpp"

namespace ff {

std::vector<std::pair<std::string, std::string>> parseProperties(xml_node<>* propGroupNode) {
	std::vector<std::pair<std::string, std::string>> properties;

	if (propGroupNode) {
		xml_node<>* propNode = propGroupNode->first_node("property");
		while (propNode) {
			char* name = propNode->first_attribute("name")->value();
			char* value = propNode->first_attribute("value")->value();

			if (name && value) {
				properties.push_back(std::make_pair(name, value));
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

void parseObjectRefs(xml_node<>* objectNode, ObjectLayerRef& objLayer) {

	while (objectNode) {
		ObjectRef obj;
		obj.set_other_objs(&objLayer.objects);

		obj.id = atoi(objectNode->first_attribute("id")->value());

		auto* type = objectNode->first_attribute("type");
		if (type && type->value()) {
			obj.type = std::hash<std::string_view>{}(type->value());
		}
		else {
			obj.type = 0u;
		}

		auto* name = objectNode->first_attribute("name");
		if (name && name->value()) {
			obj.name = name->value();
		}
		else {
			obj.name = "";
		}

		Recti area;
		area.left = atoi(objectNode->first_attribute("x")->value());
		area.top = atoi(objectNode->first_attribute("y")->value());

		auto* attr = objectNode->first_attribute("width");
		area.width = attr ? atoi(attr->value()) : 0;
		attr = objectNode->first_attribute("height");
		area.height = attr ? atoi(attr->value()) : 0;

		obj.width = area.width;
		obj.height = area.height;

		obj.position.x = area.left + area.width / 2;
		obj.position.y = area.top + area.height;

		obj.properties = parseProperties(objectNode->first_node("properties"));
		obj.points = parsePoints(objectNode->first_node("polyline"));

		objLayer.objects.push_back(std::move(obj));

		objectNode = objectNode->next_sibling();
	}
}


/////////////////////////////////////////////////////////////

LayerRef ObjectTMX::parse(xml_node<>* layerNode) {
	LayerRef layer(LayerRef::Type::Object);
	layer.id = atoi(layerNode->first_attribute("id")->value());
	parseObjectRefs(layerNode->first_node("object"), std::get<ObjectLayerRef>(layer.layer));
	return layer;
}

}