#include "fastfall/game/camera/CameraTarget.hpp"

#include "fastfall/util/id.hpp"
#include "fastfall/game/World.hpp"

namespace ff {

CameraTarget::CameraTarget(CamTargetPriority priority)
        : m_priority(priority)
{
}

Vec2f CameraTarget::get_target_pos() const { return position; }

CamTargetPriority CameraTarget::get_priority() const { return m_priority; };
CamTargetState CameraTarget::get_state() const { return m_state; };

void imgui_component(World& w, ID<CameraTarget> id) {
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

}