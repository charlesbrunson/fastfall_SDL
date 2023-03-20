#pragma once

#include "fastfall/game/systems/CollisionSystem.hpp"
#include "fastfall/game/systems/CameraSystem.hpp"
#include "fastfall/game/systems/TriggerSystem.hpp"
#include "fastfall/game/systems/SceneSystem.hpp"
#include "fastfall/game/systems/ActorSystem.hpp"
#include "fastfall/game/systems/LevelSystem.hpp"
#include "fastfall/game/systems/EmitterSystem.hpp"
#include "fastfall/game/systems/AttachSystem.hpp"
#include "fastfall/game/systems/PathSystem.hpp"

#include <tuple>
#include <variant>

namespace ff {

template<class... Ts>
struct SystemConfig {
    using Tuple = std::tuple<Ts...>;
    constexpr static size_t Count = sizeof...(Ts);
};

using Systems = SystemConfig<
    LevelSystem,
    ActorSystem,
    CollisionSystem,
    TriggerSystem,
    EmitterSystem,
    AttachSystem,
    CameraSystem,
    SceneSystem,
    PathSystem
>;

constexpr inline static std::string_view SystemNames[Systems::Count] = {
    "LevelSystem",
    "ActorSystem",
    "CollisionSystem",
    "TriggerSystem",
    "EmitterSystem",
    "AttachSystem",
    "CameraSystem",
    "SceneSystem",
    "PathSystem"
};

}