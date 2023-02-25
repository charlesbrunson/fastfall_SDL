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

}