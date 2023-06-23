#include "fastfall/game/WorldImGui.hpp"

#include "fmt/format.h"
#include <array>
#include <vector>
#include <algorithm>

#include "fastfall/game/World.hpp"

#include "fastfall/engine/input/InputSourceRealtime.hpp"
#include "fastfall/engine/input/InputSourceRecord.hpp"

#include "fastfall/game/imgui_component.hpp"

namespace ff {

std::vector<WorldImGui::WorldData> WorldImGui::worlds;
//bool WorldImGui::update_labels = false;


// ------------------------------------------------------------

void imgui_collidables(World* w) {
    for (auto [cid, col] : w->all<Collidable>()) {
        if (ImGui::TreeNode((void*)(&col), "Collidable %d", cid.value.sparse_index)) {

            ImGui::Text("Curr Pos: %3.2f, %3.2f", col.getPosition().x, col.getPosition().y);
            ImGui::Text("Prev Pos: %3.2f, %3.2f", col.getPrevPosition().x, col.getPrevPosition().y);
            ImGui::NewLine();
            ImGui::Text("Curr Center: %3.2f, %3.2f", math::rect_mid(col.getBox()).x, math::rect_mid(col.getBox()).y);
            ImGui::Text("Prev Center: %3.2f, %3.2f", math::rect_mid(col.getPrevBox()).x, math::rect_mid(col.getPrevBox()).y);
            ImGui::NewLine();
            ImGui::Text("Curr Size: %3.2f, %3.2f", col.getBox().getSize().x, col.getBox().getSize().y);
            ImGui::Text("Prev Size: %3.2f, %3.2f", col.getPrevBox().getSize().x, col.getPrevBox().getSize().y);
            ImGui::NewLine();
            ImGui::Text("Local  Vel:   %3.2f, %3.2f", col.get_local_vel().x, col.get_local_vel().y);
            ImGui::Text("Parent Vel:   %3.2f, %3.2f", col.get_parent_vel().x, col.get_parent_vel().y);
            ImGui::Text("Surface Vel:  %3.2f, %3.2f", col.get_surface_vel().x, col.get_surface_vel().y);
            ImGui::Text("Global Vel:   %3.2f, %3.2f", col.get_global_vel().x, col.get_global_vel().y);
            ImGui::Text("Last Parent Vel:   %3.2f, %3.2f", col.get_last_parent_vel().x, col.get_last_parent_vel().y);
            ImGui::NewLine();
            ImGui::Text("Accel:        %3.2f, %3.2f", col.get_acc().x, col.get_acc().y);
            ImGui::Text("Friction:     %3.2f, %3.2f", col.get_friction().x, col.get_friction().y);
            ImGui::Text("Gravity:      %3.2f, %3.2f", col.get_gravity().x, col.get_gravity().y);
            ImGui::NewLine();
            ImGui::Text("Local  Speed: %3.2f", col.get_local_vel().magnitude());
            ImGui::Text("Parent Speed: %3.2f", col.get_parent_vel().magnitude());
            ImGui::Text("Global Speed: %3.2f", col.get_global_vel().magnitude());
            ImGui::NewLine();

            if (ImGui::TreeNode((void*)(&col.tracker()), "Tracker")) {

                if (!col.tracker()) {
                    ImGui::Text("No trackers!");
                }
                else {
                    auto& tracker = *col.tracker();

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
            if (ImGui::TreeNode((void*)(&col.get_contacts()), "Collisions")) {
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

                        ImGui::Text("Velocity"); ImGui::NextColumn();
                        ImGui::Text("(%3.2f, %3.2f)", c.velocity.x, c.velocity.y); ImGui::NextColumn();

                        ImGui::Text("Surface Velocity"); ImGui::NextColumn();
                        ImGui::Text("(%3.2f, %3.2f)", c.surface_vel().x, c.surface_vel().y); ImGui::NextColumn();

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
    for (auto [id, col] : w->all<ColliderRegion>()) {
        if (ImGui::TreeNode((void *) (&col), "Collider %d", id.value.sparse_index)) {

            ImGui::Text("Curr Position: %3.2f, %3.2f", col->getPosition().x, col->getPosition().y);
            ImGui::Text("Prev Position: %3.2f, %3.2f", col->getPrevPosition().x, col->getPrevPosition().y);
            ImGui::Text("Delta Position: %3.2f, %3.2f", col->getDeltaPosition().x, col->getDeltaPosition().y);
            ImGui::Text("Velocity:      %3.2f, %3.2f", col->velocity.x, col->velocity.y);
            ImGui::Text("Delta Vel:      %3.2f, %3.2f", col->delta_velocity.x, col->delta_velocity.y);

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
    for (auto [id, trig] : w->all<Trigger>()) {
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

void imgui_camera(World* w) {

    for (auto [id, cam] : w->all<CameraTarget>()) {
        if (ImGui::TreeNode((void *) (&cam), "Camera Target %d", id.value.sparse_index)) {
            cam->get_priority();

            static std::string_view priority[] = {
                "Low", "Medium", "High"
            };

            ImGui::Text("Priority:   %s", priority[static_cast<size_t>(cam->get_priority())].data());
            ImGui::Text("State:      %s", cam->get_state() == CamTargetState::Active ? "Active" : "Inactive");
            ImGui::Text("Position:   %3.2f, %3.2f", cam->get_target_pos().x, cam->get_target_pos().y);

            ImGui::TreePop();
        }
    }
}

void imgui_scene(World* w) {
    for (auto& proxy : w->system<SceneSystem>().get_scene_order()) {
        auto& scene = w->system<SceneSystem>().config(proxy.id);

        if (ImGui::TreeNode((void *) (&scene), "Scene Object %d", proxy.id.value.sparse_index)) {
            static std::string_view priority_str[] = {
                "Lowest",
                "Low",
                "Medium",
                "High",
                "Highest"
            };

            ImGui::Text("Drawable:  %p", w->get(proxy.id));
            ImGui::Text("Layer:     %d", scene.layer_id);
            ImGui::Text("Type:      %s", scene.type == scene_type::Object ? "Object" : "Level");
            ImGui::Text("Priority:  %s", priority_str[static_cast<size_t>(scene.priority)].data());

            ImGui::Text("Enabled:   %s", scene.render_enable ? "true" : "false");
            if (ImGui::Button("Toggle Visibility")) {
                scene.render_enable = !scene.render_enable;
            }

            ImGui::TreePop();
        }
    }
}

// ------------------------------------------------------------

void entity_browser(WorldImGui::WorldData& wd)
{
    auto* w = wd.world;
    ImGui::SetNextWindowSize(ImVec2(600.f, 500.f), ImGuiCond_Once);
    if (ImGui::Begin(wd.name, &wd.show_ent_browser))
    {
        ImGui::BeginTabBar(wd.tab_name);
        if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip) || wd.tabs.empty()) {
            wd.tabs.emplace_back();
            fmt::format_to_n(wd.tabs.back().w1_name, 64, "##w1_{}", wd.tab_id_counter);
            fmt::format_to_n(wd.tabs.back().w2_name, 64, "##w2_{}", wd.tab_id_counter);
            fmt::format_to_n(wd.tabs.back().w3_name, 64, "##w3_{}", wd.tab_id_counter);
            ++wd.tab_id_counter;
        }

        unsigned tab_ndx = 0;
        for (auto& tab : wd.tabs) {

            static char ent_name[64];
            Actor* m_actor = nullptr;
            memset(ent_name, 0, 64);
            if (tab.curr_ent) {
                auto& ent = w->entities().at(*tab.curr_ent);
                m_actor = ent.actor ? w->get(*ent.actor) : nullptr;
                fmt::format_to_n(ent_name, 64, "{}{}",
                                 (m_actor && m_actor->get_actor_type() ? m_actor->get_actor_type()->name.str : "Entity"),
                                 tab.curr_ent->value.sparse_index);
            }
            else {
                fmt::format_to_n(ent_name, 64, "");
            }

            memset(tab.name, 0, 64);
            if (!tab.curr_ent) {
                fmt::format_to_n(tab.name, 64, "Empty Tab###{}", tab_ndx);
            }
            else if (!tab.curr_cmp) {
                fmt::format_to_n(tab.name, 64, "{}###{}", ent_name, tab_ndx);
            }
            else {
                fmt::format_to_n(tab.name, 64, "{}.{}###{}", ent_name, cmpid_str(*tab.curr_cmp), tab_ndx);
            }

            ImGuiTabItemFlags tab_flags = 0;
            if (tab.give_focus) {
                tab_flags |= ImGuiTabItemFlags_SetSelected;
                tab.give_focus = false;
            }
            if (ImGui::BeginTabItem(tab.name, &tab.show, tab_flags))
            {

                ImGui::Text("Entities");
                ImGui::SameLine(166.f);
                ImGui::Text("Components");
                ImGui::SameLine(166.f + 166.f);
                if (tab.curr_cmp) {
                    ImGui::Text("%s", cmpid_str(*tab.curr_cmp).c_str());
                }
                else if (tab.curr_ent && m_actor) {
                    ImGui::Text("%s", ent_name);
                }
                else {
                    ImGui::Text("%s", "");
                }

                float width =  ImGui::GetContentRegionAvail().x;
                {
                    ImGui::BeginChild(tab.w1_name, ImVec2(150.f, 0), true);
                    for (auto [id, ent] : w->entities()) {
                        bool selected = tab.curr_ent && ID<Entity>{ id.value } == *tab.curr_ent;
                        auto *actor = ent.actor ? w->get(*ent.actor) : nullptr;

                        static char sel_ent_name[64];
                        memset(sel_ent_name, 0, 64);
                        fmt::format_to_n(sel_ent_name, 64, "{}{}",
                                       (actor && actor->get_actor_type() ? actor->get_actor_type()->name.str : "Entity"),
                                       id.value.sparse_index);

                        if (ImGui::Selectable(sel_ent_name, selected)) {
                            tab.curr_cmp = std::nullopt;
                            tab.curr_ent = !selected
                                           ? std::make_optional(ID<Entity>{ id.value })
                                           : std::nullopt;
                        }
                    }
                    ImGui::EndChild();
                }
                ImGui::SameLine();
                {
                    ImGui::BeginChild(tab.w2_name, ImVec2(150.f, 0), true);
                    if (tab.curr_ent) {
                        auto& ent = w->entities().at(*tab.curr_ent);
                        for (auto &cmp_id: ent.components) {
                            bool selected = tab.curr_cmp && cmp_id == *tab.curr_cmp;
                            if (ImGui::Selectable(cmpid_str(cmp_id).c_str(), selected)) {
                                tab.curr_cmp = !selected
                                      ? std::make_optional(cmp_id)
                                      : std::nullopt;
                            }
                        }
                    }
                    ImGui::EndChild();
                }
                ImGui::SameLine();
                {
                    ImGui::BeginChild(tab.w3_name, ImVec2((std::max)(width - 300.f, 0.f), 0), true);
                    if (tab.curr_cmp) {
                        imgui_component(*w, *tab.curr_cmp);
                    }
                    else if (tab.curr_ent && m_actor) {
                        m_actor->ImGui_Inspect();
                    }
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }
            ++tab_ndx;
        }
        std::erase_if(wd.tabs, [](auto& tab) {
            return !tab.show;
        });

        ImGui::EndTabBar();
    }
    ImGui::End();
}

void imgui_entity(WorldImGui::WorldData& wd) {
    auto* w = wd.world;

    if (ImGui::Button("Show Entity Browser"))
        wd.show_ent_browser = true;
}

// ------------------------------------------------------------

void imgui_input(World* w) {
    static const char* inputNames[] = {
            "Up",
            "Left",
            "Down",
            "Right",
            "Jump",
            "Dash",
            "Attack",
            //"Mouse1",
            //"Mouse2"
    };

    ImGui::Columns(6, "inputs");
    ImGui::Separator();
    ImGui::Text("Input"); ImGui::NextColumn();
    ImGui::Text("Active"); ImGui::NextColumn();
    ImGui::Text("Magnitude"); ImGui::NextColumn();
    ImGui::Text("Counter"); ImGui::NextColumn();
    ImGui::Text("Confirmed"); ImGui::NextColumn();
    //ImGui::Text("Enabled"); ImGui::NextColumn();
    ImGui::Text("Duration"); ImGui::NextColumn();

    //ImGui::Separator();
    int i = 0;
    for (auto& [type, in] : w->input().all_inputs()) {
        ImGui::Text("%s", inputNames[i]); ImGui::NextColumn();
        ImGui::Text("%d", in.is_active()); ImGui::NextColumn();
        ImGui::Text("%d", in.magnitude()); ImGui::NextColumn();
        ImGui::Text("%d", in.num_activators()); ImGui::NextColumn();

        if (in.is_confirmable()) {
            static char confirmbuf[32];
            sprintf(confirmbuf, "Confirm##%d", i);
            if (ImGui::SmallButton(confirmbuf)) {
                w->input()[type].confirm_press();
            }
        }
        ImGui::NextColumn();
        if (in.get_last_pressed() != DBL_MAX) {
            ImGui::Text("%.2f", in.get_last_pressed());
        }
        else {
            ImGui::Text("inf");
        }
        ImGui::NextColumn();
        i++;
    };

    ImGui::Columns(1);
    ImGui::Separator();

    std::string source_str;
    auto* source = w->input().get_source();
    if (dynamic_cast<const InputSourceRecord*>(source)) {
        source_str = "record";
    }
    else if (dynamic_cast<const InputSourceRealtime*>(source)) {
        source_str = "realtime";
    }
    else if (source == nullptr) {
        source_str = "null";
    }
    else {
        source_str = "unknown";
    }

    ImGui::Text("Input Source: %s", source_str.c_str() );
    ImGui::Text("Input Tick: %zu", w->input().get_tick() );

    struct FrameData {
        size_t tick;
        InputFrame frame;
    };
    static std::vector<std::pair<FrameData, std::string>> frame_data;
    std::optional<FrameData> curr_frame;
    constexpr const size_t MAX_FRAMES = 30;


    if (auto* record = dynamic_cast<const InputSourceRecord*>(source)) {
        ImGui::Text("Record Tick: %zu", record->get_tick() );
        ImGui::Text("Record Length: %zu", record->get_record().frame_data.size() );

        if (!record->get_record().frame_data.empty()
            && record->get_tick() < record->get_record().frame_data.size())
        {
            curr_frame = {
                record->get_tick(),
                record->get_record().frame_data[record->get_tick()]
            };
        }
    }
    else if (auto* realtime = dynamic_cast<const InputSourceRealtime*>(source)) {
        ImGui::Text("Realtime Tick: %zu", realtime->get_tick() );
        ImGui::Text("Recording: %s", (realtime->is_recording() ? "Yes" : "No") );

        if (realtime->is_recording() && !realtime->get_record()->frame_data.empty()) {
            curr_frame = {
                realtime->get_tick(),
                realtime->get_record()->frame_data.back()
            };
        }
    }

    if (curr_frame) {
        if (!frame_data.empty()
            && curr_frame->tick != frame_data.back().first.tick + 1
            && curr_frame->tick != frame_data.back().first.tick)
        {
            frame_data.clear();
        }

        if (frame_data.empty()
            || curr_frame->tick == frame_data.back().first.tick + 1)
        {
            if (frame_data.size() == MAX_FRAMES) {
                frame_data.erase(frame_data.begin());
            }
            frame_data.push_back({
                 *curr_frame,
                 fmt::format("{:5} {}", curr_frame->tick, curr_frame->frame.to_string())
            });
        }
    }

    ImGui::Text("Last %zu frames:", MAX_FRAMES);
    for (auto& [frame, str] : frame_data) {
        if (frame.frame.activation_change == 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, 0xFF808080);
            ImGui::Text("%s", str.c_str());
            ImGui::PopStyleColor();
        }
        else {
            ImGui::Text("%s", str.c_str());
        }
    }
}


void imgui_status(World* w) {
    ImGui::Text("Tick Count: %zu", w->tick_count());
    ImGui::Text("Uptime: %f", w->uptime());

    ImGui::Checkbox("Lock Camera", &w->system<CameraSystem>().lockPosition);

    //const char* label, float v[2], float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags
    float v[2] = {
            w->system<CameraSystem>().currentPosition.x,
            w->system<CameraSystem>().currentPosition.y,
    };
    if (ImGui::DragFloat2("Cam Pos", v)) {
        w->system<CameraSystem>().currentPosition.x = floor(v[0]);
        w->system<CameraSystem>().currentPosition.y = floor(v[1]);
    }
    float zoom = w->system<CameraSystem>().zoomFactor;
    if (ImGui::DragFloat("Cam Zoom", &zoom, 0.01f, 0.25f, 2.f))
    {
        w->system<CameraSystem>().zoomFactor = floorf(zoom * 4.f) / 4.f;
    }
}

// ------------------------------------------------------------

WorldImGui::WorldImGui() :
        ImGuiContent(ImGuiContentType::SIDEBAR_RIGHT, "World States", "Gameplay")
{
}

void WorldImGui::ImGui_getContent()
{
    if (worlds.empty()) {
        ImGui::Text("No Instances!");
        curr_world = 0;
        return;
    }

    static char empty_world[64] = "";

    fmt::format_to_n(worlds.at(0).name, 64, "{:8} {} {}",
            fmt::ptr(worlds.at(0).world),
            (worlds.at(0).world->name.empty() ? "unnamed" : worlds.at(0).world->name), 1);

    auto& sel = worlds.empty()
            ? empty_world
            : worlds.at(curr_world).name;

    if (ImGui::BeginCombo("##Instance", sel))
    {
        unsigned sel_ndx = 0;
        for (auto& world_data : worlds)
        {
            fmt::format_to_n(world_data.name, 64, "{:8} {} {}",
                       fmt::ptr(world_data.world),
                       (world_data.world->name.empty() ? "unnamed" : world_data.world->name), sel_ndx + 1);

            const bool is_selected = (curr_world == sel_ndx);
            if (ImGui::Selectable(worlds.at(sel_ndx).name, is_selected)) {
                curr_world = sel_ndx;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
            ++sel_ndx;
        }
        ImGui::EndCombo();
    }

    auto* w = &worlds.at(curr_world);
    ImGui::Separator();

    if (ImGui::BeginTabBar("MyTabBar"))
    {
        if (ImGui::BeginTabItem("Status")) {
            imgui_status(w->world);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Entities")) {
            imgui_entity(*w);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Components")) {
            if (ImGui::BeginTabBar("Components")) {
                if (ImGui::BeginTabItem("Collidables")) {
                    imgui_collidables(w->world);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Colliders")) {
                    imgui_colliders(w->world);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Triggers")) {
                    imgui_triggers(w->world);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Cameras")) {
                    imgui_camera(w->world);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Scene Objects")) {
                    imgui_scene(w->world);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Input")) {
            imgui_input(w->world);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}


void WorldImGui::add(World* w) {
    bool exists = std::find_if(worlds.begin(), worlds.end(), [w](auto& data) {
        return data.world == w;
    }) != worlds.end();

    if (!exists) {
        worlds.emplace_back(WorldData{
            .world = w,
            //.tab_name = fmt::format("##entbrowsertabs_{}", fmt::ptr(w))
        });
        fmt::format_to_n(worlds.back().tab_name, 64, "##entbrowsertabs_{}", fmt::ptr(w));
    }
}

void WorldImGui::remove(World* w) {
    std::erase_if(worlds, [w](auto& data) {
        return data.world == w;
    });
}

void WorldImGui::ImGui_getExtraContent() {
    for (auto& wd : worlds) {
        if (wd.show_ent_browser) {
            entity_browser(wd);
        }
    }
}

}
