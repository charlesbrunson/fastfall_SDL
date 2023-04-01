#include "fastfall/game/actor/ActorType.hpp"

#include "fastfall/engine/config.hpp"

namespace ff {

uint8_t ActorInit::get_priority() const {
    return type ? type->priority : ActorType::priority_default;
}

std::optional<copyable_unique_ptr<Actor>> ActorInit::create() const {
    if (!level_object) {
        LOG_ERR_("actor init has no associated level object");
        return std::nullopt;
    }

    if (!type) {
        LOG_ERR_("actor init has no actor type");
        return std::nullopt;
    }

    if (!type->builder) {
        LOG_ERR_("actor type has no builder function");
        return std::nullopt;
    }

    auto data = *level_object;

    auto tile_size = type->tile_size;

    // test size
    if (tile_size.x > 0 && (data.area.width / TILESIZE != tile_size.x)) {
        LOG_ERR_("object width ({}) not valid for object:{:x}",
                 data.area.width, data.typehash
        );
        return std::nullopt;
    }
    if (tile_size.y > 0 && (data.area.height / TILESIZE != tile_size.y)) {
        LOG_ERR_("object height ({}) not valid for object:{:x}",
                 data.area.height, data.typehash
        );
        return std::nullopt;
    }

    // test custom properties
    for (const auto& prop : type->properties) {
        auto it = data.properties.find(prop.name);
        if (it == data.properties.end()) {
            if (prop.value) {
                data.properties.emplace(prop.name, ObjectProperty{ *prop.value }).first;
                continue;
            }
            else {
                LOG_ERR_("actor property ({}) not defined for level object:{:x}",
                         prop.name, data.typehash
                );
                return std::nullopt;
            }
        }

        if (prop.type != it->second.get_type()) {
            LOG_ERR_("actor property ({}={}) not valid for object:{:x}",
                     it->first, it->second.str_value, data.typehash
            );
            return std::nullopt;
        }
    }
    return type->builder(*this, data);
}

}