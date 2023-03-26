#include "fastfall/game/actor/ActorType.hpp"

#include "fastfall/engine/config.hpp"

namespace ff {

copyable_unique_ptr<Actor> ActorType::make_with_data(ActorInit init) const {
   if (!init.level_object) {
       LOG_ERR_("actor init has no associated level object");
       return {};
   }

   if (init.type != this) {
       LOG_ERR_("actor init type doesn't match this");
       return {};
   }

   return {};
}

}