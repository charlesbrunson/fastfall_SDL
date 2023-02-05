#include "fastfall/game/attach/AttachPoint.hpp"

#include "imgui.h"
#include "fastfall/game/World.hpp"

namespace ff {

void imgui_component(World& w, AttachPoint& cmp) {
    ImGui::Text("Curr Pos:        %3.2f, %3.2f", cmp._curr_pos.x, cmp._curr_pos.y);
    ImGui::Text("Prev Pos:        %3.2f, %3.2f", cmp._prev_pos.x, cmp._prev_pos.y);
    ImGui::Text("Curr Local Vel:  %3.2f, %3.2f", cmp._lvel.x, cmp._lvel.y);
    ImGui::Text("Prev Local Vel:  %3.2f, %3.2f", cmp._prev_lvel.x, cmp._prev_lvel.y);
    ImGui::Text("Curr Parent Vel: %3.2f, %3.2f", cmp._pvel.x, cmp._pvel.y);
    ImGui::Text("Prev Parent Vel: %3.2f, %3.2f", cmp._prev_pvel.x, cmp._prev_pvel.y);

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

AttachPoint::AttachPoint(ID<AttachPoint> t_id, Vec2f init_pos, Vec2f init_vel, Schedule sch)
    : _id(t_id)
    , sched(sch)
{
    teleport(init_pos);
    set_parent_vel(init_vel);
    set_local_vel({});
}

void AttachPoint::apply_vel(secs deltaTime) {
    _curr_pos += global_vel() * deltaTime;
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
    return _prev_pos + (_curr_pos - _prev_pos) * interp;
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

}