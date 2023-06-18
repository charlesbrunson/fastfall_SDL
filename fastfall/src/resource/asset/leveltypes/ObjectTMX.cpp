#include "ObjectTMX.hpp"

#include "fastfall/util/math.hpp"
#include "fastfall/util/log.hpp"

#include <charconv>

namespace ff {

std::map<std::string, ObjectProperty, std::less<>> parseProperties(xml_node<>* propGroupNode) {
	std::map<std::string, ObjectProperty, std::less<>> properties;

	if (propGroupNode) {
		xml_node<>* propNode = propGroupNode->first_node("property");
		while (propNode) {
			std::string_view name  = propNode->first_attribute("name")->value();
            std::string_view value = propNode->first_attribute("value")->value();
            std::string_view type  = "string";

            if (auto* node = propNode->first_attribute("type"); node && node->value()) {
                type = node->value();
            }

            size_t ndx = 0;
            for (auto& str : ObjectProperty::string) {
                if (str == type) {
                    break;
                }
                ++ndx;
            }

            auto parse_color = [](std::string_view sv) {
                uint8_t c[4];
                for (int i = 0; i < 4; ++i) {
                    std::from_chars(
                        &sv[1 + (i * 2)],
                        &sv[1 + (i * 2) + 1],
                        c[i],
                        16);
                }

                return Color {
                    c[1], c[2], c[3], c[0],
                };
            };

            ObjectProperty::Variant v;
            switch (static_cast<ObjectProperty::Type>(ndx)) {
                case ObjectProperty::Type::Bool:
                    v.emplace<bool>( value == "true" );
                    break;
                case ObjectProperty::Type::Color:
                    v.emplace<Color>(value.empty() ? parse_color(value) : Color{});
                    break;
                case ObjectProperty::Type::Float:
                    v.emplace<float>(0.f);
		    // emscripten doesn't like this :(
                    //std::from_chars(
                    //        value.data(),
                    //        value.data() + value.size(),
                    //        std::get<float>(v));
		    try {
		        float f = std::stof(value.data());
			v.emplace<float>(f);
		    }
		    catch(std::exception& e) {}

                    break;
                case ObjectProperty::Type::File:
                    v = std::filesystem::path{ value };
                    break;
                case ObjectProperty::Type::Int:
                    v.emplace<int>(0);
                    std::from_chars(
                            value.data(),
                            value.data() + value.size(),
                            std::get<int>(v));
                    break;
                case ObjectProperty::Type::Object:
                    v.emplace<ObjLevelID>();
                    std::from_chars(
                            value.data(),
                            value.data() + value.size(),
                            std::get<ObjLevelID>(v).id);
                    break;
                case ObjectProperty::Type::String:
                    v.emplace<std::string>(value);
                    break;
            }

            if (!v.valueless_by_exception()) {
                properties.emplace(name, ObjectProperty{
                    .value     = v,
                    .str_value = std::string{ value }
                });
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

		auto* type = objectNode->first_attribute("type");
		if (type && type->value()) {
            objdata.type = type->value();
			objdata.typehash = std::hash<std::string_view>{}(type->value());
		}
		else {
            objdata.type = {};
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
