#include "fastfall/game/drawable/drawable.hpp"

#include "fastfall/game/World.hpp"

#include "fastfall/render/drawable/AnimatedSprite.hpp"

namespace ff {

void imgui_component(World &w, ID<Drawable> id) {
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

}
