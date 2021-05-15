#pragma once

#include "fastfall/engine/state/EngineState.hpp"
#include "fastfall/engine/imgui/ImGuiContent.hpp"

#include "fastfall/resource/Resources.hpp"
#include "fastfall/render/AnimatedSprite.hpp"

class EmptyState : public ff::EngineState, public ff::ImGuiContent {
public:

	EmptyState() 
		: ImGuiContent(ff::ImGuiContentType::SIDEBAR_RIGHT, "EmptyState", "EmptyState")
	{
		ImGui_addContent();
		clearColor = ff::Color(0x141013FF);
		viewPos = ff::Vec2f{100.f, 100.f};

		ff::SpriteAsset* texass = ff::Resources::get<ff::SpriteAsset>("player");

		sprite.setTexture(&texass->getTexture());

		ff::Rectf texSize{ 0, 0, (float)texass->getTexture().size().x, (float)texass->getTexture().size().y };

		sprite.setTextureRect(texSize);
		sprite.setSize(texSize.getSize());


		ff::AnimID running = ff::Resources::get_animation_id("player", "running");
		asprite.set_anim(running);
	}

	void update(secs deltaTime) override {
		asprite.update(deltaTime);
	};
	void predraw(secs deltaTime) override {
		asprite.predraw(deltaTime);
	};


	void ImGui_getContent() override {
		ImGui::DragFloat2("view", &viewPos.x);

		static uint8_t min = 0;
		static uint8_t max = 255;

		ImGui::DragScalarN("bg color", ImGuiDataType_U8, &clearColor.r, 3, 1, &min, &max);
	}

	ff::AnimatedSprite asprite;
	ff::Sprite sprite;

private:

	void draw(ff::RenderTarget& target, ff::RenderState state) const override {
		target.draw(sprite);
		target.draw(asprite);
	};
};