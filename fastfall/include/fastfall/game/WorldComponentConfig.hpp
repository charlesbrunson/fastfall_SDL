#pragma once

#include <tuple>
#include <variant>


#include "fastfall/util/id_map.hpp"

#include "fastfall/game/Actor.hpp"
#include "fastfall/game/level/Level.hpp"
#include "fastfall/game/phys/Collidable.hpp"
#include "fastfall/game/phys/ColliderRegion.hpp"
#include "fastfall/game/trigger/Trigger.hpp"
#include "fastfall/game/trigger/Trigger.hpp"
#include "fastfall/game/camera/CameraTarget.hpp"
#include "fastfall/game/particle/Emitter.hpp"
#include "fastfall/game/path/PathMover.hpp"

namespace ff {

template<class... Ts>
struct ComponentConfig {
    using MapTuple = std::tuple <
    std::conditional_t<
            std::is_pointer_v < Ts>,
    poly_id_map<std::remove_pointer_t < Ts>>,
    id_map <Ts>>...
    >;
    using ComponentID = std::variant <
    ID<std::remove_pointer_t < Ts>>...
    >;
    constexpr static size_t Count = sizeof...(Ts);
};

using Components = ComponentConfig<
    Actor*,
    Collidable,
    ColliderRegion*,
    Trigger,
    CameraTarget*,
    Drawable*,
    Emitter,
    AttachPoint,
    PathMover,
    TileLayer
>;

}

