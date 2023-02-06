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
        std::string name = "Empty Tab";
        std::optional<ID<Entity>>  curr_ent;
        std::optional<ComponentID> curr_cmp;

        std::string w1_name;
        std::string w2_name;
        std::string w3_name;
    };
    struct WorldData {
        World* world;
        std::string name;
        bool show_ent_browser = false;
        std::vector<EntBrowserTab> tabs;
        std::string tab_name;
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
