#include "fastfall/user_types.hpp"

namespace ff {

std::map<std::string, user_types::EngineStateFactory, std::less<>>
user_types::engine_state_types;

std::map<size_t, const ActorType*>
user_types::actor_types;

std::map<std::string, TileMaterial, std::less<>>
user_types::tile_material_types;

std::map<std::string, user_types::TileLogicFactory, std::less<>>
user_types::tile_logic_types;

bool user_types::register_tile_material(TileMaterial mat) {
    if (tile_material_types.contains(mat.typeName)) {
        return false;
    }
    tile_material_types.emplace(mat.typeName, std::move(mat));
    return true;
}

user_types::EngineStateFactory user_types::get_engine_state_factory(std::string_view name) {
    auto it = engine_state_types.find(name);
    return it != engine_state_types.end() ? it->second : nullptr;
}

const ActorType* user_types::get_actor_type(std::string_view name) {
    return get_actor_type(std::hash<std::string_view>{}(name));
}
const ActorType* user_types::get_actor_type(size_t type_hash) {
    auto it     = actor_types.find(type_hash);
    return it != actor_types.end() ? it->second : nullptr;
}

user_types::TileLogicFactory user_types::get_tile_logic_factory(std::string_view name) {
    auto it = tile_logic_types.find(name);
    return it != tile_logic_types.end() ? it->second : nullptr;
}

const TileMaterial& user_types::get_tile_material(std::string_view name) {
    auto it = tile_material_types.find(name);
    return it != tile_material_types.end() ? it->second : TileMaterial::standard;
}

}