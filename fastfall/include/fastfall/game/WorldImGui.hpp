#pragma once

#include "fastfall/engine/imgui/ImGuiContent.hpp"

#include "fastfall/game/Entity.hpp"
#include "fastfall/game/ComponentID.hpp"

#include <vector>

namespace ff {

class World;

class WorldImGui : public ImGuiContent {
public:
    struct EntBrowserTab {
        bool show = true;
        bool give_focus = false;
        char name[64] = "Empty Tab";
        std::optional<ID<Entity>>  curr_ent;
        std::optional<ComponentID> curr_cmp;

        char w1_name[64];
        char w2_name[64];
        char w3_name[64];
    };
    struct WorldData {
        World* world;
        char name[64];
        bool show_ent_browser = false;
        std::vector<EntBrowserTab> tabs;
        char tab_name[64];
    };

    WorldImGui();

    void ImGui_getContent() override;

    void ImGui_getExtraContent() override;

    static void add(World* w);
    static void remove(World* w);

    static std::vector<WorldData> worlds;
private:
    unsigned curr_world = 0;

};

}
