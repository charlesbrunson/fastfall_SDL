#include "fastfall/game/actor/ActorType.hpp"

#include "fastfall/engine/config.hpp"

#include "fastfall/game/actor/Actor.hpp"

namespace ff {

uint8_t ActorInit::get_priority() const {
    return type ? type->priority : ActorType::priority_default;
}

ActorInit& ActorInit::type_or(const ActorType* n_type) {
    type = (type ? type : n_type);
    return *this;
}

copyable_unique_ptr<Actor> ActorInit::create() const {
    if (!level_object) {
        LOG_ERR_("actor init has no associated level object");
        return {};
    }

    if (!type) {
        LOG_ERR_("actor init has no actor type");
        return {};
    }

    if (!type->builder) {
        LOG_ERR_("actor type has no builder function");
        return {};
    }

    auto data = *level_object;

    auto tile_size = type->tile_size;

    // test size
    if (tile_size.x > 0 && (data.area.width / TILESIZE != tile_size.x)) {
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
                return {};
            }
        }

        if (prop.type != it->second.get_type()) {
            LOG_ERR_("actor property ({}={}) not valid for object:{:x}",
                     it->first, it->second.str_value, data.typehash
            );
            return {};
        }
    }
    return type->builder(*this, data);
}

}