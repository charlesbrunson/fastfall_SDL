#include "fastfall/game/WorldImGui.hpp"

#include "fmt/format.h"
#include <array>

#include "fastfall/game/World.hpp"

namespace ff {

std::vector<World*> WorldImGui::worlds;

WorldImGui::WorldImGui() :
        ImGuiContent(ImGuiContentType::SIDEBAR_RIGHT, "World", "World")
{
}

void WorldImGui::ImGui_getContent()
{
    if (worlds.empty()) {
        ImGui::Text("No Instances!");
        curr_world = nullptr;
        return;
    }

    static std::array<std::string, 32> world_labels = { "" };
    static unsigned comboWorld = 0;

    int i = 0;
    for (auto w : worlds)
    {
        world_labels[i] = fmt::format("{:8}", fmt::ptr(w));

        if (++i > 32)
            break;
    }
    while (i < 32) {
        if (!world_labels[i].empty())
            world_labels[i] = "";

        ++i;
    }

    if (ImGui::BeginCombo("##Instance", world_labels[comboWorld].c_str())) {

        for (int n = 0; n < 32; n++)
        {
            if (world_labels[n].empty())
                continue;

            const bool is_selected = (comboWorld == n);
            if (ImGui::Selectable(world_labels[n].c_str(), is_selected)) {
                comboWorld = n;
                curr_world = worlds[n];
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    //ImGui::SameLine();
    //if (ImGui::Button("Reset")) {
    //    Instance(instanceID)->want_reset = true;
    //}

    if (!curr_world) {
        if (!worlds.empty())
            curr_world = worlds[0];
        else
            return;
    }

    ImGui::Separator();

    if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_AutoSelectNewTabs))
    {
        if (ImGui::BeginTabItem("Status")) {
            // TODO
            ImGui::Text("Tick Count: %zu", curr_world->tick_count());
            ImGui::Text("Uptime: %f", curr_world->uptime());
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Levels")) {
            // TODO
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Objects")) {
            // TODO
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Components")) {
            // TODO
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Systems")) {
            // TODO
            ImGui::EndTabItem();
        }

        //if (ImGui::BeginTabItem("Collision")) {
        //    //collisionContent(instanceID);
        //    ImGui::EndTabItem();
        //}
        //if (ImGui::BeginTabItem("Objects")) {
        //    //objectContent(instanceID);
        //    ImGui::EndTabItem();
        //}
        //if (ImGui::BeginTabItem("Level")) {
        //    //levelContent(instanceID);
        //    ImGui::EndTabItem();
        //}
        //if (ImGui::BeginTabItem("Camera")) {
        //    //cameraContent(instanceID);
        //    ImGui::EndTabItem();
        //}
        ImGui::EndTabBar();
    }
}

void WorldImGui::ImGui_getExtraContent() {

    if (curr_world) {
        for (auto& obj : curr_world->all<GameObject>()) {
            if (obj->m_show_inspect
                && ImGui::Begin(obj->type().type.name.c_str(), &obj->m_show_inspect))
            {
                obj->ImGui_Inspect();
                ImGui::End();
            }
        }
    }
}

}