#pragma once

#include "fastfall/engine/state/EngineState.hpp"
#include "fastfall/engine/imgui/ImGuiContent.hpp"

#include "fastfall/resource/Resources.hpp"
#include "fastfall/game/Instance.hpp"

class EmptyState : public ff::EngineState, public ff::ImGuiContent {
public:

	EmptyState() 
		: ImGuiContent(ff::ImGuiContentType::SIDEBAR_RIGHT, "EmptyState", "EmptyState")
	{
		ImGui_addContent();
		clearColor = ff::Color(0x141013FF);
		viewPos = ff::Vec2f{100.f, 100.f};

		instance = ff::CreateInstance();
		instanceID = instance->getInstanceID();
		assert(instance);

		ff::LevelAsset* lvlptr = ff::Resources::get<ff::LevelAsset>("map_test");
		assert(lvlptr);
		if (lvlptr) {
			instance->addLevel(*lvlptr);

			ff::Level* lvl = instance->getActiveLevel();
			assert(lvl);
			if (lvl) {

				if (!lvl->getFGLayers().empty()) {
					//lvl->getFGLayers().begin()->getCollisionMap()->teleport(Vec2f(32.f, 32.f));
					instance->getCollision().addColliderRegion(lvl->getFGLayers().begin()->getCollisionMap());

					//platform_pos = Vec2f(9.f * TILESIZE_F, 15.f * TILESIZE_F);
					//platform1 = std::make_shared<ColliderSimple>(Rectf(6.f * TILESIZE_F, 15.f * TILESIZE_F, 3.f * TILESIZE_F, 1.f * TILESIZE_F));
					//platform2 = std::make_shared<ColliderSimple>(Rectf(42.f * TILESIZE_F, 13.f * TILESIZE_F, 3.f * TILESIZE_F, 1.f * TILESIZE_F));
					//platform->teleport(platform_pos);
					//instance->getCollision().addColliderRegion(std::shared_ptr<ColliderSimple>(platform1));
					//instance->getCollision().addColliderRegion(std::shared_ptr<ColliderSimple>(platform2));
				}

				float xf = static_cast<float>(lvl->size().x);
				float yf = static_cast<float>(lvl->size().y);
				//viewPos = ff::Vec2f(xf, yf) * TILESIZE_F / 2.f;

				//clearColor = lvl->getBGColor();
				//clearColor = ff::Color{ 0x141013FF };
			}
		}

		instance->getActiveLevel()->update(0.0);
		instance->getObject().update(0.0);
		instance->getCollision().update(0.0);

		stateID = ff::EngineStateID::TEST_STATE;
	}
	~EmptyState()
	{
		ff::DestroyInstance(instance->getInstanceID());
	}

	void update(secs deltaTime) override {

		if (deltaTime > 0.0) {
			auto* tile = &*instance->getActiveLevel()->getFGLayers().begin();
			auto colmap = tile->getCollisionMap().get();

			static secs timebuf = 0.0;
			timebuf += deltaTime;
			//platform1->setPosition(Vec2f(128.f * cos(timebuf * 2.f), 0.f)); //64.f * sin(timebuf)));
			//platform2->setPosition(Vec2f(64.f * cos(timebuf * 2.f), 64.f * sin(timebuf * 2.f))); //64.f * sin(timebuf)));

			colmap->setPosition(colmap->getPosition());
		}

		instance->getActiveLevel()->update(deltaTime);
		instance->getObject().update(deltaTime);
		instance->getCollision().update(deltaTime);
		instance->getCamera().update(deltaTime);

		viewZoom = instance->getCamera().zoomFactor;
	};
	void predraw(secs deltaTime) override {

		instance->getObject().predraw(deltaTime);
		//instance->getCollision().predraw(deltaTime);
		instance->getActiveLevel()->predraw(deltaTime);
		//viewPos = instance->getCamera().currentPosition;

		//if (deltaTime > 0.0)
		//	debug_draw::swapDrawLists();
	};


	void ImGui_getContent() override {
		ImGui::DragFloat2("view", &viewPos.x);

		static uint8_t min = 0;
		static uint8_t max = 255;

		ImGui::DragScalarN("bg color", ImGuiDataType_U8, &clearColor.r, 3, 1, &min, &max);
	}

private:

	ff::GameInstance* instance;

	void draw(ff::RenderTarget& target, ff::RenderState state) const override {
		auto size = instance->getActiveLevel()->size() * TILESIZE_F;

		ff::ShapeRectangle lvlRect(
			ff::Rectf(ff::Vec2f(), ff::Vec2f(size)),
			instance->getActiveLevel()->getBGColor()			
			);
		target.draw(lvlRect, state);

		for (auto& bg : instance->getActiveLevel()->getBGLayers()) {
			target.draw(bg, state);
		}

		//target.draw(instance->getObject().getObjectDrawList(false));

		target.draw(instance->getObject().getObjectDrawList(false));

		bool firstFG = true;
		for (auto& fg : instance->getActiveLevel()->getFGLayers()) {

			target.draw(fg, state);

			if (firstFG) {
				firstFG = false;
				target.draw(instance->getObject().getObjectDrawList(true));
			}
		}

	};
};