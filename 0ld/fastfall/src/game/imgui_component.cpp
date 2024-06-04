#include "fastfall/game/imgui_component.hpp"

#include "fastfall/game/World.hpp"
#include "imgui.h"

#include "fastfall/render/drawable/AnimatedSprite.hpp"
#include "fastfall/render/drawable/VertexArray.hpp"
#include "fastfall/game/drawable/drawable.hpp"

#include "fastfall/game/WorldImGui.hpp"

namespace ff {
    void imgui_component(World& w, ComponentID id) {
        std::visit([&w]<typename T>(ID<T>& id) {
            imgui_component(w, id);
        }, id);
    }

    void imgui_component_ref(const World& w, const ComponentID &cmp) {
        if(ImGui::Selectable(fmt::format("<{}>", cmpid_str(cmp)).c_str())) {
            auto ent_id = w.entity_of(cmp);
            auto& ent = w.entities().at(ent_id);

            auto it = std::find_if(
                    WorldImGui::worlds.begin(), WorldImGui::worlds.end(),
                    [&w](auto& wd) { return wd.world == &w; });

            if (it != WorldImGui::worlds.end())
            {
                for (auto& tab : it->tabs) {
                    if (tab.curr_cmp == cmp) {
                        tab.give_focus = true;
                        return;
                    }
                    else if (tab.curr_ent == ent_id) {
                        tab.curr_cmp = cmp;
                        tab.give_focus = true;
                        return;
                    }
                }

                it->show_ent_browser = true;
                it->tabs.push_back({
                    .show = true,
                    .give_focus = true,
                    .curr_ent = ent_id,
                    .curr_cmp = cmp
                });
                fmt::format_to_n(it->tabs.back().w1_name, 64, "##w1_{}", it->tab_id_counter);
                fmt::format_to_n(it->tabs.back().w2_name, 64, "##w2_{}", it->tab_id_counter);
                fmt::format_to_n(it->tabs.back().w3_name, 64, "##w3_{}", it->tab_id_counter);
                ++it->tab_id_counter;
            }
        }
    }

}