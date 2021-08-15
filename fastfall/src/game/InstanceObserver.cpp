#include "fastfall/game/InstanceObserver.hpp"

#include "fastfall/game/Instance.hpp"

#include <array>

#include "fastfall/util/log.hpp"

#include "fastfall/game/phys/collision/Contact.hpp"
#include "fastfall/render/DebugDraw.hpp"

namespace ff {

unsigned comboInstance = 0u;
InstanceID instanceID{ 1u };

void collisionContent(GameContext context) {
	CollisionManager* man = &context.collision().get();//&Instance(instance)->getCollision();

	// TODO
	
	/*
	static bool draw_collider = true;
	if (ImGui::Checkbox("Draw Colliders", &draw_collider))
		debug_draw::setTypeEnabled(debug_draw::Type::COLLISION_COLLIDER, draw_collider);

	static bool draw_collidable = true;
	if (ImGui::Checkbox("Draw Collidables", &draw_collidable))
		debug_draw::setTypeEnabled(debug_draw::Type::COLLISION_COLLIDABLE, draw_collidable);

	static bool draw_contact = true;
	if (ImGui::Checkbox("Draw Contacts", &draw_contact))
		debug_draw::setTypeEnabled(debug_draw::Type::COLLISION_CONTACT, draw_contact);

	static bool draw_raycast = true;
	if (ImGui::Checkbox("Draw Raycasts", &draw_raycast))
		debug_draw::setTypeEnabled(debug_draw::Type::COLLISION_RAYCAST, draw_raycast);
	*/

	

	ImGui::Text("Colliders");
	//...

	ImGui::Separator();
	ImGui::Text("Collidables");

	auto& collidables = man->get_collidables();
	for (auto& coldata : collidables) {
		auto& col = coldata.collidable;

		if (ImGui::TreeNode((void*)(&col), "Collidable %d", col.get_ID().value)) {

			ImGui::Text("Curr Pos: %3.2f, %3.2f", col.getPosition().x, col.getPosition().y);

			ImGui::Text("Curr Center: %3.2f, %3.2f", math::rect_mid(col.getBox()).x, math::rect_mid(col.getBox()).y);
			ImGui::Text("Prev Center: %3.2f, %3.2f", math::rect_mid(col.getPrevBox()).x, math::rect_mid(col.getPrevBox()).y);

			ImGui::Text("Curr Size: %3.2f, %3.2f", col.getBox().getSize().x, col.getBox().getSize().y);
			ImGui::Text("Prev Size: %3.2f, %3.2f", col.getPrevBox().getSize().x, col.getPrevBox().getSize().y);


			ImGui::Text("Velocity:     %3.2f, %3.2f", col.get_vel().x, col.get_vel().y);
			ImGui::Text("Speed:        %3.2f", col.get_vel().magnitude());
			ImGui::Text("Gravity:      %3.2f, %3.2f", col.get_gravity().x, col.get_gravity().y);

			if (ImGui::TreeNode((void*)(&col.get_trackers()), "Tracker")) {

				if (col.get_trackers().empty()) {
					ImGui::Text("No trackers!");
				}

				for (auto& tracker : col.get_trackers()) {


					static char labelbuf[32];
					sprintf(labelbuf, "Friction (%d)", tracker->settings.has_friction);

					if (ImGui::SmallButton(labelbuf)) {
						tracker->settings.has_friction = !tracker->settings.has_friction;
					} ImGui::SameLine();
					sprintf(labelbuf, "Platform Stick (%d)", tracker->settings.move_with_platforms);
					if (ImGui::SmallButton(labelbuf)) {
						tracker->settings.move_with_platforms = !tracker->settings.move_with_platforms;
					} ImGui::SameLine();
					sprintf(labelbuf, "Slope Stick (%d)", tracker->settings.slope_sticking);
					if (ImGui::SmallButton(labelbuf)) {
						tracker->settings.slope_sticking = !tracker->settings.slope_sticking;
					}ImGui::SameLine();
					sprintf(labelbuf, "Wall stop (%d)", tracker->settings.slope_wall_stop);
					if (ImGui::SmallButton(labelbuf)) {
						tracker->settings.slope_wall_stop = !tracker->settings.slope_wall_stop;
					}

					static char trackerbuf[32];
					sprintf(trackerbuf, "%p", &col);
					ImGui::Columns(2, trackerbuf);
					ImGui::SetColumnWidth(0, 120.f);
					ImGui::Separator();

					ImGui::Text("Angle Range"); ImGui::NextColumn();
					ImGui::Text("(%3.2f, %3.2f)", tracker->get_angle_min().degrees(), tracker->get_angle_max().degrees()); ImGui::NextColumn();

					ImGui::Text("Has Contact"); ImGui::NextColumn();
					ImGui::Text("%s", tracker->has_contact() ? "true" : "false"); ImGui::NextColumn();

					ImGui::Text("Contact Duration"); ImGui::NextColumn();
					ImGui::Text("%3.2f", tracker->get_contact_time()); ImGui::NextColumn();

					ImGui::Text("Air Duration"); ImGui::NextColumn();
					ImGui::Text("%3.2f", tracker->get_air_time()); ImGui::NextColumn();

					ImGui::Text("Traversal Speed"); ImGui::NextColumn();
					ImGui::Text("%3.2f", tracker->traverse_get_speed()); ImGui::NextColumn();

					if (tracker->get_contact().has_value()) {
						const PersistantContact& c = tracker->get_contact().value();

						ImGui::Text("Surface Normal"); ImGui::NextColumn();
						ImGui::Text("(%3.2f, %3.2f)", c.collider_normal.x, c.collider_normal.y); ImGui::NextColumn();

						ImGui::Text("Ortho Normal"); ImGui::NextColumn();
						ImGui::Text("(%1.f, %1.f)", c.ortho_normal.x, c.ortho_normal.y); ImGui::NextColumn();

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
						ImGui::Text("%s", contactTypeToString(c.type)); ImGui::NextColumn();

					}
					ImGui::Columns(1);
					ImGui::Separator();
				}

				ImGui::TreePop();
			}
			if (ImGui::TreeNode((void*)(&col), "Collisions")) {

				bool has_collision = false;

				//const auto& arbiter = coldata.regionArbiters;

				//const auto& arbiter = man->getArbiters()->find(&col);

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
						ImGui::Text("(%3.2f, %3.2f)", c.collider_normal.x, c.collider_normal.y); ImGui::NextColumn();

						ImGui::Text("Ortho Normal"); ImGui::NextColumn();
						ImGui::Text("(%1.f, %1.f)", c.ortho_normal.x, c.ortho_normal.y); ImGui::NextColumn();

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
						ImGui::Text("%s", contactTypeToString(c.type)); ImGui::NextColumn();

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
	auto& colliders = man->get_colliders();
	for (auto& col : colliders) {
		if (ImGui::TreeNode((void*)(&col), "Collider %d", col->get_ID().value)) {

			ImGui::Text("Curr Position: %3.2f, %3.2f", col->getPosition().x, col->getPosition().y);
			ImGui::Text("Prev Position: %3.2f, %3.2f", col->getPrevPosition().x, col->getPrevPosition().y);


			float pos[2] = { col->getPosition().x, col->getPosition().y };

			if (ImGui::DragFloat2("Set Pos", pos)) {
				col->setPosition(Vec2f(pos[0], pos[1]));
			}
			ImGui::TreePop();
		}
	}

}

void levelContent(GameContext context) {

	constexpr auto displayTileLayer = [](TileLayer& tile) {
		ImGui::Checkbox("Hidden", &tile.hidden);
		ImGui::Text("Collision = %s", tile.hasCollision ? "true" : "false");
		ImGui::Text("Parallax = %s", tile.isParallax() ? "true" : "false");
		ImGui::Text("Scrolling = %s", tile.hasScrollX() || tile.hasScrollY() ? "true" : "false");




	};
	constexpr auto displayObjectRef = [](ObjectRef& obj) {
		//ImGui::Checkbox("Hidden", &layer.hidden);

		ImGui::Text("Name : \"%s\"", obj.name.c_str());
		ImGui::Text("Type ID : %d", obj.id);
		ImGui::Text("Position : (%4d, %4d)", obj.position.x, obj.position.y);
		ImGui::Text("Size : (%2d, %2d)", obj.width, obj.height);

		if (!obj.properties.empty()) {
			if (ImGui::TreeNode("Properties")) {
				for (auto& prop : obj.properties) {
					ImGui::Text("%s : %s", prop.first.c_str(), prop.second.c_str());
				}
				ImGui::TreePop();
			}
		}
		if (!obj.points.empty()) {
			if (ImGui::TreeNode("Points")) {
				unsigned pCount = 0;
				for (auto& p : obj.points) {
					ImGui::Text("points[%2d] : (% 4d, % 4d)", pCount++, p.x, p.y);

				}
				ImGui::TreePop();
			}
		}
	};


	for (auto& lvlpair : context.levels().get_all()) { // Instance(instance)->getAllLevels()) {

		Level* lvl = lvlpair.second.get();

		if (ImGui::CollapsingHeader(lvl->name()->c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) {

			ImGui::Text("Instance = %u", (unsigned int)lvl->getInstanceID());
			ImGui::Text("Dimensions = %u, %u", lvl->size().x, lvl->size().y);
			ImGui::Text("BG Color = (%3u, %3u, %3u)", lvl->getBGColor().r, lvl->getBGColor().g, lvl->getBGColor().b);

			int i = 0;
			for (TileLayer& layer : lvl->getBGLayers()) {
				if (ImGui::TreeNode((void*)(&layer), "%d. Background  #%u", i, layer.getID())) {
					displayTileLayer(layer);

					ImGui::TreePop();
				}
				i++;
			}
			
			ObjectLayer& layer = lvl->getObjLayer();
			if (ImGui::TreeNode((void*)(&layer), "%d. Objects  #%u", i, layer.getLayerID())) {
				for (auto& obj : layer.getLayerRef()->objLayer->objects) {

					const std::string* type = GameObjectLibrary::lookupTypeName(obj.second.type);
					if (ImGui::TreeNode((char*)&obj, "%s #%u", (type ? type->c_str() : "Anonymous Type"), obj.second.id)) {

						if (ImGui::BeginChild((char*)&obj, ImVec2(0, 100), true)) {
							displayObjectRef(obj.second);
						}
						ImGui::EndChild();
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
			i++;
			
			for (TileLayer& layer : lvl->getFGLayers()) {
				if (ImGui::TreeNode((void*)(&layer), "%d. Foreground%s #%u", i, layer.hasCollision ? "*" : " ", layer.getID())) {

					displayTileLayer(layer);

					ImGui::TreePop();
				}
				i++;
			}
		}
	}
}

void objectContent(GameContext context) {
	GameObjectManager* man = &context.objects().get(); //&Instance(instance)->getObject();

	ImGui::Text("Object Count: %3lu", man->getObjects().size());

	ImGui::Columns(2, NULL, false);
	for (auto& obj : man->getObjects()) {

		ImGui::Text("%s #%u", obj->getTypeName().c_str(), obj->getID());
		ImGui::NextColumn();

		static char buttonBuf[32];
		sprintf(buttonBuf, "%s##%d", obj->showInspect ? "Uninspect" : " Inspect ", obj->getID());
		if (ImGui::Button(buttonBuf)) {
			obj->showInspect = !obj->showInspect;
		}
		ImGui::NextColumn();
	}
	ImGui::Columns(1);

}
void cameraContent(GameContext context) {
	//GameCamera* man = &Instance(instance)->getCamera();

	GameCamera* man = &context.camera().get();

	ImGui::Text("Center (%3.2f, %3.2f)", man->currentPosition.x, man->currentPosition.y);
	ImGui::Checkbox("Lock Position", &man->lockPosition);

	float pos[2] = { man->currentPosition.x, man->currentPosition.y };

	if (ImGui::DragFloat2("Set Pos", pos)) {
		man->currentPosition = Vec2f(pos[0], pos[1]);
	}


	auto& targets = man->getTargets();

	ImGui::Separator();
	for (unsigned i = 0; i < targets.size(); i++) {
		auto& target = targets[i];


		if (target.type == GameCamera::TargetType::MOVING) {
			ImGui::Text("Target: Moving(%3.2f, %3.2f)", target.movingTarget->x, target.movingTarget->y);
		}
		else if (target.type == GameCamera::TargetType::STATIC) {
			ImGui::Text("Target: Static(%3.2f, %3.2f)", target.staticTarget.x, target.staticTarget.y);
		}
		else {
			ImGui::Text("Target: Invalid");
		}
		ImGui::Text("Offset (%3.2f, %3.2f)", target.offset.x, target.offset.y);
		ImGui::Text("Target Priority: %u", target.priority);

		ImGui::Separator();
	}
	if (ImGui::SliderFloat("Zoom", &man->zoomFactor, 0.25f, 3.f, "%.2f")) {
		man->zoomFactor = roundf(man->zoomFactor * 4.f) / 4.f;
	}


	ImGui::SameLine();
	if (ImGui::Button("Reset")) {
		man->zoomFactor = 1.f;
	}
}


InstanceObserver::InstanceObserver() :
	ImGuiContent(ImGuiContentType::SIDEBAR_RIGHT, "Game Instance", "Instance")
{

}

void InstanceObserver::ImGui_getContent() {
	std::map<InstanceID, GameInstance>& instances = AllInstances();

	if (instances.empty()) {
		ImGui::Text("No Instances!");
		return;
	}

	static std::array<std::string, 32> instanceLabel = { "" };

	int i = 0;
	for (auto& inst : instances) {

		instanceLabel[i] = "Instance #" + std::to_string(inst.first);

		i++;
		if (i > 32)
			break;
	}

	if (ImGui::BeginCombo("##Instance", instanceLabel[comboInstance].c_str())) {

		for (int n = 0; n < 32; n++)
		{
			if (instanceLabel[n].empty())
				continue;

			const bool is_selected = (comboInstance == n);
			if (ImGui::Selectable(instanceLabel[n].c_str(), is_selected)) {
				comboInstance = n;
				instanceID = InstanceID{ comboInstance + 1u };
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::SameLine();
	if (ImGui::Button("Reset")) {

		//sf::Clock check;

		Instance(instanceID)->reset();
		//LOG_INFO("Reset Instance: {}ms", check.getElapsedTime().asMilliseconds());

	}

	ImGui::Separator();

	if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_AutoSelectNewTabs))
	{

		if (ImGui::BeginTabItem("Collision")) {
			collisionContent(instanceID);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Objects")) {
			objectContent(instanceID);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Level")) {
			levelContent(instanceID);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Camera")) {
			cameraContent(instanceID);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void InstanceObserver::ImGui_getExtraContent() {

	GameContext context{ instanceID };

	if (context.valid()) {
		for (auto& obj : context.objects().get().getObjects()) {
			if (obj->showInspect && ImGui::Begin(obj->getTypeName().c_str(), &obj->showInspect)) {
				obj->ImGui_Inspect();
				ImGui::End();
			}
		}
	}

}

}