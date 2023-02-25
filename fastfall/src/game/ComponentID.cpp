#include "fastfall/game/ComponentID.hpp"

#include "fmt/format.h"

namespace ff {

    std::string cmpid_str(const ComponentID& cmp) {
        return std::visit([ndx = cmp.index()]<class T>(const ID<T>& id) -> std::string {
                return fmt::format(
                    "{}{}",
                    ComponentNames[ndx],
                    id.value.sparse_index);
            }, cmp);
    }

}