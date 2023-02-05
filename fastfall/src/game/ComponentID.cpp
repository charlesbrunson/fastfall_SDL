#include "fastfall/game/ComponentID.hpp"
#include "imgui.h"
#include "fmt/format.h"

#include "fastfall/game/World.hpp"

#include <string>

namespace ff {

    std::string cmpid_str(const ComponentID& cmp) {
        return std::visit([ndx = cmp.index()]<class T>(const ID<T>& id) -> std::string {
                return fmt::format(
                    "{} {}:{}",
                    ComponentID_Str[ndx],
                    id.value.sparse_index,
                    id.value.generation);
            }, cmp);
    }

    void imgui_component_ref(const World& w, const ComponentID &cmp) {
        if(ImGui::Selectable(fmt::format("[{}]", cmpid_str(cmp)).c_str())) {
            auto& ent = w.entities().at(w.entity_of(cmp));
            ent.imgui.cmp_selected  = cmp;
            ent.imgui.imgui_show    = true;
        }
    }
}