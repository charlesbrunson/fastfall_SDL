#include "fastfall/game/actor/ActorType.hpp"

#include "fastfall/engine/config.hpp"

namespace ff {

uint8_t ActorInit::get_priority() const {
    return type ? type->priority : ActorType::priority_default;
}

std::pair<std::string, std::string> ActorProperty::to_string_pair() const
{
    std::string v = "";
    if (value) {
        switch (type)
        {
        case ActorPropertyType::String:
            v = std::get<std::string>(*value);
            break;
        case ActorPropertyType::Int:
            v = fmt::format("{}", std::get<int>(*value));
            break;
        case ActorPropertyType::Bool:
            v = fmt::format("{}", std::get<bool>(*value));
            break;
        case ActorPropertyType::Float:
            v = fmt::format("{}", std::get<float>(*value));
            break;
        case ActorPropertyType::Object:
            v = fmt::format("{}", std::get<ObjLevelID>(*value).id);
            break;
        }
    }
    return std::make_pair(name, v);
}

bool test_value(ActorPropertyType type, const std::string& value) {
    try {
        switch(type) {
        case ActorPropertyType::String:
            break;
        case ActorPropertyType::Int:
            std::stoi(value);
        case ActorPropertyType::Bool:
            if (value != "true" && value != "false") {
                throw std::exception{};
            }
        case ActorPropertyType::Float:
            std::stof(value);
        case ActorPropertyType::Object:
            if (std::stoi(value) < 0)
                throw std::exception();
        }
    }
    catch (std::exception except) {
        return false;
    }
    return true;
}

copyable_unique_ptr<Actor> ActorType::make_with_data(ActorInit init) const {
    if (!init.level_object) {
        LOG_ERR_("actor init has no associated level object");
        return {};
    }

    if (init.type != this) {
        LOG_ERR_("actor init type not this instance: {} to {}", name.str, init.type->name.str);
        return {};
    }

    if (!builder) {
        LOG_ERR_("no builder function");
        return {};
    }

    auto data = *init.level_object;

    // test size
    if (tile_size.x > 0 && (init.level_object->area.width / TILESIZE != tile_size.x)) {
        LOG_ERR_("object width ({}) not valid for object:{:x}",
                 data.area.width, data.typehash
        );
        return {};
    }
    if (tile_size.y > 0 && (data.area.height / TILESIZE != tile_size.y)) {
        LOG_ERR_("object height ({}) not valid for object:{:x}",
                 data.area.height, data.typehash
        );
        return {};
    }

    // test custom properties
    for (const auto& prop : properties) {
        auto it = data.properties.find(prop.name);
        if (it == data.properties.end()) {
            if (prop.value) {
                data.properties.emplace(prop.to_string_pair()).first;
                continue;
            }
            else {
                LOG_ERR_("actor property ({}) not defined for level object:{:x}",
                         prop.name, data.typehash
                );
                return {};
            }
        }

        if (!test_value(prop.type, it->second)) {
            LOG_ERR_("actor property ({}={}) not valid for object:{:x}",
                     it->first, it->second, data.typehash
            );
            return {};
        }

    }
    return builder(init, data);
}

}