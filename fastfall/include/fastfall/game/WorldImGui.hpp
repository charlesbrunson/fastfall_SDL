#pragma once

#include "fastfall/engine/imgui/ImGuiContent.hpp"

#include <vector>

namespace ff {

class World;

class WorldImGui : public ImGuiContent {
private:
    World* curr_world = nullptr;
    static std::vector<World*> worlds;

public:
    WorldImGui();

    void ImGui_getContent() override;

    void ImGui_getExtraContent() override;

    static void add(World* w) {
        bool exists = std::find(worlds.begin(), worlds.end(), w) != worlds.end();

        if (!exists)
            worlds.push_back(w);
    }

    static void remove(World* w) {
        std::erase(worlds, w);
    }

};

}
