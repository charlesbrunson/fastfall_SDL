#include "fastfall/game/imgui_component.hpp"

#include "fastfall/game/World.hpp"
#include "imgui.h"

#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/render/VertexArray.hpp"

namespace ff {
    void imgui_component(World&w , ID<AttachPoint> id) {
        auto& cmp = w.at(id);

        auto cpos = cmp.curr_pos();
        auto ppos = cmp.prev_pos();
        auto cvel = cmp.local_vel();
        auto pvel = cmp.prev_local_vel();
        auto cpvel = cmp.parent_vel();
        auto ppvel = cmp.prev_parent_vel();

        ImGui::Text("Curr Pos:        %3.2f, %3.2f", cpos.x, cpos.y);
        ImGui::Text("Prev Pos:        %3.2f, %3.2f", ppos.x, ppos.y);
        ImGui::Text("Curr Local Vel:  %3.2f, %3.2f", cvel.x, cvel.y);
        ImGui::Text("Prev Local Vel:  %3.2f, %3.2f", pvel.x, pvel.y);
        ImGui::Text("Curr Parent Vel: %3.2f, %3.2f", cpvel.x, cpvel.y);
        ImGui::Text("Prev Parent Vel: %3.2f, %3.2f", ppvel.x, ppvel.y);

        auto& sys = w.system<AttachSystem>();
        auto& attachs = sys.get_attachments(cmp.id());

        auto parent = sys.get_attachpoint(cmp.id());
        ImGui::Text("Has Constraint: %s", (cmp.constraint ? "Yes" : "No"));
        ImGui::Text("Parent: ");
        ImGui::SameLine();
        if (parent) {
            imgui_component_ref(w, parent.value());
        }
        else {
            ImGui::Text("None");
        }

        if (ImGui::TreeNode((void*)(&attachs), "Attached Components (%d)", (unsigned)attachs.size())) {
            ImGui::Columns(2);
            for (auto& cmp_attach : attachs) {
                imgui_component_ref(w, cmp_attach.id);
                ImGui::NextColumn();
                ImGui::Text("%3.2f, %3.2f", cmp_attach.offset.x, cmp_attach.offset.y);
                ImGui::NextColumn();
            }
            ImGui::Columns();
            ImGui::TreePop();
        }
    }

    void imgui_component(World&w , ID<CameraTarget> id) {
        static constexpr std::string_view priority_str[] = {
                "Low",
                "Medium",
                "High"
        };

        auto &cmp = w.at(id);
        auto pos = cmp.get_target_pos();
        auto state = cmp.get_state();
        auto priority = cmp.get_priority();
        ImGui::Text("Position: %3.2f, %3.2f", pos.x, pos.y);
        ImGui::Text("State:    %s", state == CamTargetState::Active ? "Active" : "Inactive");
        ImGui::Text("Priority: %s", priority_str[static_cast<unsigned>(priority)].data());
    }

    void imgui_component(World&w , ID<TileLayer> id) {

    }

    void imgui_component(World&w , ID<Emitter> id) {

    }

    void imgui_component(World&w , ID<PathMover> id) {

        auto& cmp = w.at(id);
        auto attach_id = cmp.get_attach_id();

        ImGui::Text("Attach ID: ");
        ImGui::SameLine();
        imgui_component_ref(w, attach_id);

        ImGui::Text("At Start: %s", cmp.at_start() ? "Yes" : "No");
        ImGui::Text("At End: %s", cmp.at_end() ? "Yes" : "No");
        auto prev_point = cmp.prev_waypoint_pos();
        ImGui::Text("Prev Point: %3.2f, %3.2f", prev_point.x, prev_point.y);
        auto next_point = cmp.next_waypoint_pos();
        ImGui::Text("Next Point: %3.2f, %3.2f", next_point.x, next_point.y);
        auto pos = cmp.get_pos();
        ImGui::Text("Curr Pos: %3.2f, %3.2f", pos.x, pos.y);
        auto vel = cmp.get_vel();
        ImGui::Text("Curr Vel: %3.2f, %3.2f", vel.x, vel.y);
        auto path_offset = cmp.get_path_offset();
        ImGui::Text("Path offset: %3.2f, %3.2f", path_offset.x, path_offset.y);
        auto is_stopped = cmp.is_stopped();
        ImGui::Text("Is Stopped: %s", is_stopped ? "Yes" : "No");
        auto& path = cmp.get_path();
        if (ImGui::TreeNode((void*)(&path), "Path"))
        {
            if (ImGui::TreeNode((void *) (&path.waypoints), "Path Points (%d)", (unsigned)path.waypoints.size())) {
                for (auto &point: path.waypoints) {
                    ImGui::Text("%3.2f, %3.2f", point.x, point.y);
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }

    void imgui_component(World&w , ID<Collidable> id) {
        auto& col = w.at(id);

        auto text_vec2 = [](std::string_view label, const auto& v) {
            ImGui::Text("%s: ", label.data());     ImGui::NextColumn();
            ImGui::Text("%3.2f, %3.2f", v.x, v.y); ImGui::NextColumn();
        };
        auto text_vec1 = [](std::string_view label, const auto& v) {
            ImGui::Text("%s: ", label.data());     ImGui::NextColumn();
            ImGui::Text("%3.2f", v); ImGui::NextColumn();
        };

        ImGui::Columns(2);
        text_vec2("Curr Pos",       col.getPosition());
        text_vec2("Prev Pos",       col.getPosition());
        text_vec2("Curr Center",    col.getBox().center());
        text_vec2("Prev Center",    col.getPrevBox().center());
        text_vec2("Curr Size",      col.getBox().getSize());
        text_vec2("Prev Size",      col.getPrevBox().getSize());

        text_vec2("Local  Vel",      col.get_local_vel());
        text_vec2("Parent Vel",      col.getPrevBox().getSize());
        text_vec2("Surface Vel",     col.get_parent_vel());
        text_vec2("Global Vel",      col.get_global_vel());

        text_vec2("Last Parent Vel",      col.get_last_parent_vel());
        text_vec2("Accel",       col.get_acc());
        text_vec2("Friction",      col.get_friction());
        text_vec2("Gravity",       col.get_gravity());

        text_vec1("Local  Speed",       col.get_local_vel().magnitude());
        text_vec1("Parent Speed",       col.get_parent_vel().magnitude());
        text_vec1("Global Speed",       col.get_global_vel().magnitude());

        ImGui::Text("Attach ID: "); ImGui::NextColumn();
        imgui_component_ref(w, col.get_attach_id()); ImGui::NextColumn();

        text_vec2("Attach Offset",       col.get_attach_origin());
        ImGui::Columns();

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
                //ImGui::SetColumnWidth(0, 120.f);
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

                    ImGui::Text("Collider"); ImGui::NextColumn();
                    if (tracker.get_contact()->id) {
                        imgui_component_ref(w, tracker.get_contact()->id->collider);
                    }
                    else {
                        ImGui::Text("N/A");
                    }
                    ImGui::NextColumn();

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
                        //ImGui::SetColumnWidth(0, 120.f);
                        ImGui::Separator();

                        has_collision = true;
                    }

                    ImGui::Text("Collider: "); ImGui::NextColumn();
                    if (c.id && c.id->collider) {
                        imgui_component_ref(w, ComponentID{ c.id->collider });
                    }
                    else {
                        ImGui::Text("Combined");
                    }
                    ImGui::NextColumn();

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

            ImGui::Columns();
            ImGui::TreePop();
        }
    }

    void imgui_component(World&w , ID<ColliderRegion> id) {
        auto& col = w.at(id);
        ImGui::Text("Curr Position:  %3.2f, %3.2f",     col.getPosition().x, col.getPosition().y);
        ImGui::Text("Prev Position:  %3.2f, %3.2f", col.getPrevPosition().x, col.getPrevPosition().y);
        ImGui::Text("Delta Position: %3.2f, %3.2f", col.getDeltaPosition().x, col.getDeltaPosition().y);
        ImGui::Text("Velocity:       %3.2f, %3.2f", col.velocity.x, col.velocity.y);
        ImGui::Text("Delta Vel:      %3.2f, %3.2f", col.delta_velocity.x, col.delta_velocity.y);

        float pos[2] = { col.getPosition().x, col.getPosition().y };

        if (ImGui::DragFloat2("Set Pos", pos)) {
            col.setPosition(Vec2f(pos[0], pos[1]));

            Vec2f nVel = (col.getPosition() - col.getPrevPosition()) / (1.0 / 60.0);
            col.delta_velocity = nVel - col.velocity;
            col.velocity = nVel;
        }
    }

    void imgui_component(World&w , ID<Drawable> id) {
        auto &cmp = w.at(id);
        auto& cfg = w.system<SceneSystem>().config(id);

        static constexpr std::string_view ScenePriority_str[] = {
            "Lowest",
            "Low",
            "Normal",
            "High",
            "Highest"
        };

        ImGui::Checkbox("Visible", &cfg.render_enable);
        ImGui::Text("Scene Type:     %s", cfg.type == scene_type::Object ? "Object" : "Level");
        ImGui::Text("Scene Priority: %s", ScenePriority_str[static_cast<unsigned>(cfg.priority)].data());
        ImGui::Text("Scene Layer:    %d", cfg.layer_id);
        ImGui::Text("Curr Pos: %3.2f, %3.2f", cfg.curr_pos.x, cfg.curr_pos.y);
        ImGui::Text("Prev Pos: %3.2f, %3.2f", cfg.prev_pos.x, cfg.prev_pos.y);
        if (ImGui::TreeNode((void*)(&cfg.rstate), "Render State")) {
            auto tex = cfg.rstate.texture.get();
            ImGui::Text("Texture ID: %d", tex->getID());
            ImGui::Text("Shader ID:  %d", cfg.rstate.program->getID());
            auto pos = cfg.rstate.transform.getPosition();
            ImGui::Text("Offset: %3.2f, %3.2f", pos.x, pos.y);
            auto origin = cfg.rstate.transform.getScale();
            ImGui::Text("Origin: %3.2f, %3.2f", origin.x, origin.y);
            auto scale = cfg.rstate.transform.getScale();
            ImGui::Text("Scale:  %3.2f, %3.2f", scale.x, scale.y);
            ImGui::TreePop();
        }
        ImGui::Separator();
        if (auto* animspr = dynamic_cast<AnimatedSprite*>(&cmp)) {
            ImGui::Text("Drawable Type: Animated Sprite");
            if (auto* anim = animspr->get_anim()) {
                ImGui::Text("Animation: %16s - %-16s", anim->get_sprite_name().data(), anim->anim_name.data());
                auto* tex = Resources::get<SpriteAsset>(anim->get_sprite_name());
                ImGui::Text("Texture:   %s", tex ? (const char*)tex->get_texture_path().c_str() : "None");
            }
            else {
                ImGui::Text("Animation: %s", "None");
                ImGui::Text("Texture:   %s", "None");
            }
            auto pos = animspr->get_pos();
            ImGui::Text("Sprite Pos: %3.2f, %3.2f", pos.x, pos.y);
            ImGui::Text("Frame: %d", animspr->get_frame());
            ImGui::Text("HFlip: %s", animspr->get_hflip() ? "Yes" : "No");
            ImGui::Text("Playback rate: %3.2f", animspr->get_playback());
        }
        else if (auto* va = dynamic_cast<VertexArray*>(&cmp)) {
            ImGui::Text("Drawable Type: Vertex Array");
            ImGui::Text("Vertex Count: %d", (unsigned)va->size());
        }
        else {
            ImGui::Text("Drawable Type: Unknown");
        }
    }

    void imgui_component(World&w , ID<Trigger> id) {
        static constexpr std::string_view State_str[] = {
                "None",
                "Loop",
                "Entry",
                "Exit"
        };
        static constexpr std::string_view  Overlap_str[] = {
                "Partial",
                "Outside",
                "Inside"
        };

        auto &cmp = w.at(id);

        bool enabled = cmp.is_enabled();
        bool active = cmp.is_activated();
        Rectf area = cmp.get_area();

        auto& overlap = cmp.overlap;
        auto& drivers = cmp.drivers;
        auto& self_flags = cmp.self_flags;
        auto& filter_flags = cmp.filter_flags;

        ImGui::Text("Enabled: %s", enabled ? "Yes" : "No");
        ImGui::Text("Active:  %s", active  ? "Yes" : "No");
        ImGui::Text("Area: (%3.2f, %3.2f, %3.2f, %3.2f)",
                    area.left, area.top, area.width, area.height);

        ImGui::Text("Overlap: %s", Overlap_str[static_cast<unsigned>(overlap)].data());
        if (ImGui::TreeNode((void*)(&self_flags), "%s %d", "Self tags:", (unsigned)self_flags.size())) {
            for (auto &tag: self_flags) {
                ImGui::Text("%s", tag.tag_name_str().data());
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode((void*)(&filter_flags), "%s %d", "Filter tags:", (unsigned)filter_flags.size())) {
            for (auto &tag: filter_flags) {
                ImGui::Text("%s", tag.tag_name_str().data());
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode((void*)(&drivers), "Drivers: %d", (unsigned)drivers.size())) {
            for (auto &[id, driver]: drivers) {
                imgui_component_ref(w, id);
                ImGui::Text("\tState: %s", State_str[static_cast<unsigned>(driver.state)].data());
                ImGui::Text("\tDuration: %f", driver.duration.time);
            }
            ImGui::TreePop();
        }
    }

    void imgui_component(World& w, ComponentID id) {
        std::visit([&w](auto& id) {
            if constexpr (requires (World& xw, decltype(id) x) { imgui_component(xw, x); } ) {
                imgui_component(w, id);
            }
            else {
                ImGui::Text("No imgui_component() overload.");
            }
        }, id);
    }

    void imgui_component_ref(const World& w, const ComponentID &cmp) {
        if(ImGui::Selectable(fmt::format("[{}]", cmpid_str(cmp)).c_str())) {
            auto& ent = w.entities().at(w.entity_of(cmp));

            //ent.imgui.cmp_selected  = cmp;
            //ent.imgui.imgui_show    = true;
        }
    }

}