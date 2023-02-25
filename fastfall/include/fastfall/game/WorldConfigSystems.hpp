#pragma once

#include "fastfall/game/CollisionSystem.hpp"
#include "fastfall/game/CameraSystem.hpp"
#include "fastfall/game/TriggerSystem.hpp"
#include "fastfall/game/SceneSystem.hpp"
#include "fastfall/game/ActorSystem.hpp"
#include "fastfall/game/LevelSystem.hpp"
#include "fastfall/game/EmitterSystem.hpp"
#include "fastfall/game/AttachSystem.hpp"
#include "fastfall/game/PathSystem.hpp"

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