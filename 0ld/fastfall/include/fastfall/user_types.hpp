#pragma once

#include "fastfall/engine/state/EngineState.hpp"
#include "fastfall/game/actor/Actor.hpp"
#include "fastfall/game/tile/Tile.hpp"
#include "fastfall/game/level/TileLogic.hpp"

namespace ff {

    class World;

    class user_types {
        using EngineStateFactory = std::unique_ptr<EngineState>(*)();
        static std::map<std::string, EngineStateFactory, std::less<>> engine_state_types;

        static std::map<size_t, const ActorType*> actor_types;

        static std::map<std::string, TileMaterial, std::less<>> tile_material_types;

        using TileLogicFactory = copyable_unique_ptr<TileLogic>(*)(World&);
        static std::map<std::string, TileLogicFactory, std::less<>> tile_logic_types;

    public:
        template<std::derived_from<EngineState> T>
        static bool register_engine_state(std::string name) {
            if (engine_state_types.contains(name))
                return false;

            engine_state_types[name] = []() -> std::unique_ptr<EngineState> {
                return std::make_unique<T>();
            };
            return true;
        }

        template<std::derived_from<Actor> T>
        static bool register_actor() {
            const auto& type = T::actor_type;
            if (actor_types.contains(type.name.hash))
            {
                return false;
            }
            actor_types.emplace(type.name.hash, &type);
            return true;
        }

        template<std::derived_from<TileLogic> T>
        static bool register_tile_logic(std::string name) {
            if (tile_logic_types.contains(name))
                return false;

            tile_logic_types[name] = [](World& w) -> copyable_unique_ptr<TileLogic> {
                return make_copyable_unique<TileLogic, T>(w);
            };
            return true;
        }

        static bool register_tile_material(TileMaterial mat);

        static EngineStateFactory   get_engine_state_factory(std::string_view name);
        static const ActorType*     get_actor_type(std::string_view name);
        static const ActorType*     get_actor_type(size_t type_hash);
        static TileLogicFactory     get_tile_logic_factory(std::string_view name);
        static const TileMaterial&  get_tile_material(std::string_view name);
    };
}