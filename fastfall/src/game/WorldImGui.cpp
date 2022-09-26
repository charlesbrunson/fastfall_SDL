#include "fastfall/game/WorldImGui.hpp"

#include "fmt/format.h"
#include <array>

#include "fastfall/game/World.hpp"

namespace ff {

std::vector<World*> WorldImGui::worlds;
bool WorldImGui::update_labels = false;


// ------------------------------------------------------------

void imgui_collidables(World* w) {
    for (auto& col : w->all<Collidable>()) {
        auto id = *w->id_of(col);
        if (ImGui::TreeNode((void*)(&col), "Collidable %d", id.value.sparse_index)) {

            ImGui::Text("Curr Pos: %3.2f, %3.2f", col.getPosition().x, col.getPosition().y);

            ImGui::Text("Curr Center: %3.2f, %3.2f", math::rect_mid(col.getBox()).x, math::rect_mid(col.getBox()).y);
            ImGui::Text("Prev Center: %3.2f, %3.2f", math::rect_mid(col.getPrevBox()).x, math::rect_mid(col.getPrevBox()).y);

            ImGui::Text("Curr Size: %3.2f, %3.2f", col.getBox().getSize().x, col.getBox().getSize().y);
            ImGui::Text("Prev Size: %3.2f, %3.2f", col.getPrevBox().getSize().x, col.getPrevBox().getSize().y);


            ImGui::Text("Velocity:     %3.2f, %3.2f", col.get_vel().x, col.get_vel().y);
            ImGui::Text("Accel:        %3.2f, %3.2f", col.get_acc().x, col.get_acc().y);
            ImGui::Text("Friction:     %3.2f, %3.2f", col.get_friction().x, col.get_friction().y);
            ImGui::Text("Speed:        %3.2f", col.get_vel().magnitude());
            ImGui::Text("Gravity:      %3.2f, %3.2f", col.get_gravity().x, col.get_gravity().y);

            if (ImGui::TreeNode((void*)(&col.get_trackers()), "Tracker")) {

                if (col.get_trackers().empty()) {
                    ImGui::Text("No trackers!");
                }

                for (auto& tracker : col.get_trackers()) {


                    static char labelbuf[32];
                    sprintf(labelbuf, "Friction (%d)", tracker.settings.has_friction);

                    if (ImGui::SmallButton(labelbuf)) {
                        tracker.settings.has_friction = !tracker.settings.has_friction;
                    } ImGui::SameLine();
                    sprintf(labelbuf, "Platform Stick (%d)", tracker.settings.move_with_platforms);
                    if (ImGui::SmallButton(labelbuf)) {
                        tracker.settings.move_with_platforms = !tracker.settings.move_with_platforms;
                    } ImGui::SameLine();
                    sprintf(labelbuf, "Slope Stick (%d)", tracker.settings.slope_sticking);
                    if (ImGui::SmallButton(labelbuf)) {
                        tracker.settings.slope_sticking = !tracker.settings.slope_sticking;
                    }ImGui::SameLine();
                    sprintf(labelbuf, "Wall stop (%d)", tracker.settings.slope_wall_stop);
                    if (ImGui::SmallButton(labelbuf)) {
                        tracker.settings.slope_wall_stop = !tracker.settings.slope_wall_stop;
                    }

                    static char trackerbuf[32];
                    sprintf(trackerbuf, "%p", &col);
                    ImGui::Columns(2, trackerbuf);
                    ImGui::SetColumnWidth(0, 120.f);
                    ImGui::Separator();

                    ImGui::Text("Angle Range"); ImGui::NextColumn();
                    ImGui::Text("(%3.2f, %3.2f)", tracker.angle_range.min.degrees(), tracker.angle_range.max.degrees()); ImGui::NextColumn();

                    ImGui::Text("Has Contact"); ImGui::NextColumn();
                    ImGui::Text("%s", tracker.has_contact() ? "true" : "false"); ImGui::NextColumn();

                    ImGui::Text("Contact Duration"); ImGui::NextColumn();
                    ImGui::Text("%3.2f", tracker.contact_time); ImGui::NextColumn();

                    ImGui::Text("Air Duration"); ImGui::NextColumn();
                    ImGui::Text("%3.2f", tracker.air_time); ImGui::NextColumn();

                    ImGui::Text("Traversal Speed"); ImGui::NextColumn();
                    auto tspeed = tracker.traverse_get_speed();
                    ImGui::Text("%3.2f", tspeed ? *tspeed : 0.f); ImGui::NextColumn();

                    ImGui::Text("Max Speed"); ImGui::NextColumn();
                    ImGui::Text("%3.2f", tracker.settings.max_speed); ImGui::NextColumn();

                    if (tracker.get_contact().has_value()) {
                        const auto& c = tracker.get_contact().value();

                        ImGui::Text("Surface Normal"); ImGui::NextColumn();
                        ImGui::Text("(%3.2f, %3.2f)", c.collider_n.x, c.collider_n.y); ImGui::NextColumn();

                        ImGui::Text("Ortho Normal"); ImGui::NextColumn();
                        ImGui::Text("(%1.f, %1.f)", c.ortho_n.x, c.ortho_n.y); ImGui::NextColumn();

                        ImGui::Text("Separation"); ImGui::NextColumn();
                        ImGui::Text("%s%3.3f", (c.separation == 0 ? " " : (c.separation > 0 ? "+" : "-")), c.separation); ImGui::NextColumn();

                        ImGui::Text("Surface Velocity"); ImGui::NextColumn();
                        ImGui::Text("(%3.2f, %3.2f)", c.velocity.x, c.velocity.y); ImGui::NextColumn();

                        ImGui::Text("Has Contact"); ImGui::NextColumn();
                        ImGui::Text("%s", c.hasContact ? "true" : "false"); ImGui::NextColumn();

                        ImGui::Text("Impact Time"); ImGui::NextColumn();
                        if (c.hasImpactTime) {
                            ImGui::Text("%.3f", c.impactTime);
                        }
                        else {
                            ImGui::Text("N/A");
                        } ImGui::NextColumn();

                        ImGui::Text("Duration"); ImGui::NextColumn();
                        ImGui::Text("%3.2f", c.touchDuration); ImGui::NextColumn();

                        ImGui::Text("Contact Type"); ImGui::NextColumn();
                        ImGui::Text("%s", contactTypeToString(c.type).data()); ImGui::NextColumn();

                    }
                    ImGui::Columns(1);
                    ImGui::Separator();
                }

                ImGui::TreePop();
            }
            if (ImGui::TreeNode((void*)(&col), "Collisions")) {
                bool has_collision = false;
                if (!col.get_contacts().empty()) {

                    for (auto& c : col.get_contacts()) {

                        if (!has_collision) {

                            static char collisionbuf[32];
                            sprintf(collisionbuf, "%p", &col);
                            ImGui::Columns(2, collisionbuf);
                            ImGui::SetColumnWidth(0, 120.f);
                            ImGui::Separator();

                            has_collision = true;
                        }

                        ImGui::Text("Surface Normal"); ImGui::NextColumn();
                        ImGui::Text("(%3.2f, %3.2f)", c.collider_n.x, c.collider_n.y); ImGui::NextColumn();

                        ImGui::Text("Ortho Normal"); ImGui::NextColumn();
                        ImGui::Text("(%1.f, %1.f)", c.ortho_n.x, c.ortho_n.y); ImGui::NextColumn();

                        ImGui::Text("Separation"); ImGui::NextColumn();
                        ImGui::Text("%s%3.3f", (c.separation == 0 ? " " : (c.separation > 0 ? "+" : "-")), c.separation); ImGui::NextColumn();

                        ImGui::Text("Surface Velocity"); ImGui::NextColumn();
                        ImGui::Text("(%3.2f, %3.2f)", c.velocity.x, c.velocity.y); ImGui::NextColumn();

                        ImGui::Text("Has Contact"); ImGui::NextColumn();
                        ImGui::Text("%s", c.hasContact ? "true" : "false"); ImGui::NextColumn();

                        ImGui::Text("Impact Time"); ImGui::NextColumn();
                        if (c.hasImpactTime) {
                            ImGui::Text("%.3f", c.impactTime);
                        }
                        else {
                            ImGui::Text("N/A");
                        } ImGui::NextColumn();

                        ImGui::Text("Duration"); ImGui::NextColumn();
                        ImGui::Text("%3.2f", c.touchDuration); ImGui::NextColumn();


                        ImGui::Text("Contact Type"); ImGui::NextColumn();
                        ImGui::Text("%s", contactTypeToString(c.type).data()); ImGui::NextColumn();

                        ImGui::Separator();
                    }
                }
                else {
                    ImGui::Text("None"); ImGui::NextColumn(); ImGui::NextColumn();
                    ImGui::Separator();
                }

                if (!has_collision) {
                    ImGui::Text("No collisions!");
                }

                ImGui::Columns(1);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }
}

void imgui_colliders(World* w) {
    auto& colliders = w->all<ColliderRegion>();
    for (auto& col : colliders) {
        auto id = colliders.id_of(col);
        if (ImGui::TreeNode((void *) (&col), "Collider %d", id.value.sparse_index)) {

            ImGui::Text("Curr Position: %3.2f, %3.2f", col->getPosition().x, col->getPosition().y);
            ImGui::Text("Prev Position: %3.2f, %3.2f", col->getPrevPosition().x, col->getPrevPosition().y);
            ImGui::Text("Velocity:      %3.2f, %3.2f", col->velocity.x, col->velocity.y);

            float pos[2] = {col->getPosition().x, col->getPosition().y};

            if (ImGui::DragFloat2("Set Pos", pos)) {
                col->setPosition(Vec2f(pos[0], pos[1]));

                Vec2f nVel = (col->getPosition() - col->getPrevPosition()) / (1.0 / 60.0);
                col->delta_velocity = nVel - col->velocity;
                col->velocity = nVel;
            }
            ImGui::TreePop();
        }
    }
}

void imgui_triggers(World* w) {
    auto &triggers = w->all<Trigger>();
    for (auto &trig: triggers) {
        auto id = triggers.id_of(trig);
        if (ImGui::TreeNode((void *) (&trig), "Trigger %d", id.value.sparse_index)) {

            ImGui::Text("Enabled: %s", trig.is_enabled() ? "true" : "false");
            ImGui::Text("Active: %s", trig.is_activated() ? "true" : "false");

            switch (trig.overlap) {
                case Trigger::Overlap::Partial: ImGui::Text("Overlap: Partial"); break;
                case Trigger::Overlap::Outside: ImGui::Text("Overlap: Outside"); break;
                case Trigger::Overlap::Inside:  ImGui::Text("Overlap: Inside");  break;
            }

            ImGui::Separator();
            ImGui::Text("self flags:");
            for(auto& self_flag : trig.self_flags) {
                ImGui::Text("\t%s", self_flag.tag_name_str().data());
            }
            ImGui::Separator();
            ImGui::Text("filter flags:");
            for(auto& filter : trig.filter_flags) {
                ImGui::Text("\t%s", filter.tag_name_str().data());
            }
            ImGui::Separator();
            ImGui::Text("drivers:");
            for(auto& driver : trig.drivers) {
                ImGui::Text("\tTrigger %d", driver.first.value.sparse_index);
            }
            ImGui::Separator();

            ImGui::TreePop();
        }
    }
}

// ------------------------------------------------------------

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

    if (update_labels) {
        int i = 0;
        for (auto w: worlds) {
            world_labels[i] = fmt::format("{:8} {}", fmt::ptr(w), (i + 1));

            if (++i > 32)
                break;
        }
        while (i < 32) {
            world_labels[i] = "";
            ++i;
        }
        update_labels = false;
    }

    if (ImGui::BeginCombo("##Instance", world_labels[comboWorld].c_str())) {

        for (int n = 0; n < 32; n++)
        {
            if (world_labels[n].empty())
                break;

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

    if (!curr_world) {
        if (!worlds.empty())
            curr_world = worlds[0];
        else
            return;
    }

    ImGui::Separator();

    if (ImGui::BeginTabBar("MyTabBar"))
    {
        if (ImGui::BeginTabItem("Status")) {
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
            if (ImGui::BeginTabBar("Components")) {
                if (ImGui::BeginTabItem("Collidables")) {
                    imgui_collidables(curr_world);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Colliders")) {
                    imgui_colliders(curr_world);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Triggers")) {
                    imgui_triggers(curr_world);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Cameras")) {

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Scene Objects")) {

                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            // TODO
            ImGui::EndTabItem();
        }

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