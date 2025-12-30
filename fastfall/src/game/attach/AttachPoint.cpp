#include "fastfall/game/attach/AttachPoint.hpp"

#include "imgui.h"
#include "fastfall/game/World.hpp"
#include "fastfall/game/imgui_component.hpp"

namespace ff {

void imgui_component(World& w, AttachPoint& cmp) {
}

AttachPoint::AttachPoint(ID<AttachPoint> t_id, Vec2f init_pos, Vec2f init_vel)
    : _id(t_id)
{
    teleport(init_pos);
    set_parent_vel(init_vel);
    set_local_vel({});
}

void AttachPoint::apply_vel(secs deltaTime) {
    _curr_pos += global_vel() * static_cast<float>(deltaTime);
}

void AttachPoint::set_pos(Vec2f next_pos) {
    _curr_pos = next_pos;
}

void AttachPoint::teleport(Vec2f next_pos) {
    _curr_pos = next_pos;
    _prev_pos = next_pos;
}

void AttachPoint::move(Vec2f offset) {
    _curr_pos += offset;
}

void AttachPoint::shift(Vec2f offset) {
    _curr_pos += offset;
    _prev_pos += offset;
}

Vec2f AttachPoint::interpolate(float interp) const {
    return math::lerp(_prev_pos, _curr_pos, interp);
}

Vec2f AttachPoint::curr_pos() const {
    return _curr_pos;
}

Vec2f AttachPoint::prev_pos() const {
    return _prev_pos;
}

void AttachPoint::update_prev() {
    _prev_pos = _curr_pos;
    _prev_lvel = _lvel;
    _prev_pvel = _pvel;
}

ID<AttachPoint> AttachPoint::id() const {
    return _id;
}

void imgui_component(World& w, ID<AttachPoint> id) {
    auto& cmp = w.at(id);
    auto& sys = w.system<AttachSystem>();

    auto cpos  = cmp.curr_pos();
    auto ppos  = cmp.prev_pos();
    auto cvel  = cmp.local_vel();
    auto pvel  = cmp.prev_local_vel();
    auto cpvel = cmp.parent_vel();
    auto ppvel = cmp.prev_parent_vel();

    ImGui::Text("Is root:         %s", sys.is_attachpoint_root(id) ? "Yes" : "No");
    ImGui::Text("Curr Pos:        %3.2f, %3.2f", cpos.x, cpos.y);
    ImGui::Text("Prev Pos:        %3.2f, %3.2f", ppos.x, ppos.y);
    ImGui::Text("Curr Local Vel:  %3.2f, %3.2f", cvel.x, cvel.y);
    ImGui::Text("Prev Local Vel:  %3.2f, %3.2f", pvel.x, pvel.y);
    ImGui::Text("Curr Parent Vel: %3.2f, %3.2f", cpvel.x, cpvel.y);
    ImGui::Text("Prev Parent Vel: %3.2f, %3.2f", ppvel.x, ppvel.y);

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
        ImGui::Columns(2, NULL, false);
        int counter = 0;
        for (auto& [cmp_id, data] : attachs) {
            imgui_component_ref(w, cmp_id);
            ImGui::NextColumn();
            static float v[2];
            ImGui::PushID(counter);
            v[0] = data.offset.x;
            v[1] = data.offset.y;
            if (ImGui::DragFloat2("", v)) {
                if (auto* cfg = sys.get_attach_config(cmp.id()))
                {
                    cfg->offset = Vec2f{ v[0], v[1] };
                }
            }
            ImGui::PopID();
            ImGui::NextColumn();
            ++counter;
        }
        ImGui::Columns();
        ImGui::TreePop();
    }
}

}