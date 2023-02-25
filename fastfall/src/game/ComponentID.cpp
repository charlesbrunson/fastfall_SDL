#include "fastfall/game/ComponentID.hpp"
#include "imgui.h"
#include "fmt/format.h"

#include "fastfall/game/World.hpp"

#include <string>

namespace ff {

    std::string cmpid_str(const ComponentID& cmp) {
        return std::visit([]<class T>(const ID<T>& id) -> std::string {
                return fmt::format(
                    "{}{}",
                    typeid(T).name(),
                    //ComponentID_Str[ndx],
                    id.value.sparse_index);
            }, cmp);
    }

}