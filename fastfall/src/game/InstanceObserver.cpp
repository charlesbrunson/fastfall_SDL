#include "fastfall/game/InstanceObserver.hpp"

#include "fastfall/game/Instance.hpp"

#include <array>

#include "fastfall/util/log.hpp"

#include "fastfall/game/phys/collision/Contact.hpp"
#include "fastfall/render/DebugDraw.hpp"

#include "fastfall/game/InstanceInterface.hpp"

namespace ff {

unsigned comboInstance = 0u;
InstanceID instanceID{ 1u };

void collisionContent(GameContext context) {
	//CollisionManager* man = &context.collision().get();//&Instance(instance)->getCollision();

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

	auto& collidables = *instance::phys_get_collidables(context); //man->get_collidables();
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

					ImGui::Text("Max Speed"); ImGui::NextColumn();
					ImGui::Text("%3.2f", tracker->settings.max_speed); ImGui::NextColumn();

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
	auto& colliders = *instance::phys_get_colliders(context);;
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
		ImGui::Text("Collision = %s", tile.has_collision() ? "true" : "false");
		ImGui::Text("Parallax = %s", tile.isParallax() ? "true" : "false");
		ImGui::Text("Scrolling = %s", tile.isScroll() ? "true" : "false");
	};

	constexpr auto displayObjectRef = [](const ObjectRef& obj) {
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


	for (auto& lvlpair : *instance::lvl_get_all(context)) { // Instance(instance)->getAllLevels()) {

		Level* lvl = lvlpair.second.get();

		if (ImGui::CollapsingHeader(lvl->name().c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) {

			ImGui::Text("Instance = %u", (unsigned int)lvl->getInstanceID());
			ImGui::Text("Dimensions = %u, %u", lvl->size().x, lvl->size().y);
			ImGui::Text("BG Color = (%3u, %3u, %3u)", lvl->getBGColor().r, lvl->getBGColor().g, lvl->getBGColor().b);

			constexpr const char* bg_label  = "%2d. Background            #%u";
			constexpr const char* obj_label = "%2d. Objects               #%u";
			constexpr const char* fg_label  = "%2d. Foreground%s           #%u";

			auto it = lvl->getTileLayers().begin();
			for (; it != lvl->getTileLayers().end() && it->position < 0; it++) {
				if (ImGui::TreeNode(
					(void*)(&*it), 
					bg_label, 
					it->position, 
					it->tilelayer.getID())) 
				{
					displayTileLayer(it->tilelayer);

					ImGui::TreePop();
				}
			}
			ObjectLayer& layer = lvl->getObjLayer();
			if (ImGui::TreeNode(
				(void*)(&layer), 
				obj_label, 
				0, 
				layer.getLayerID())) 
			{
				for (auto& obj : layer.getObjectRefs()) {

					const std::string* type = GameObjectLibrary::lookupTypeName(obj.type);
					if (ImGui::TreeNode((char*)&obj, "%s #%u", (type ? type->c_str() : "Anonymous Type"), obj.id)) {

						if (ImGui::BeginChild((char*)&obj, ImVec2(0, 100), true)) {
							displayObjectRef(obj);
						}
						ImGui::EndChild();
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
			for (; it != lvl->getTileLayers().end(); it++) {
				if (ImGui::TreeNode(
					(void*)(&layer), 
					fg_label, 
					it->position, 
					it->tilelayer.has_collision() ? "*" : " ", 
					it->tilelayer.getID())) 
				{

					displayTileLayer(it->tilelayer);

					ImGui::TreePop();
				}
			}

		}
	}
}

void objectContent(GameContext context) {
	const GameObjectManager* man = instance::obj_get_man(context); //&Instance(instance)->getObject();

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

	//GameCamera* man = &context.camera().get();
	const GameCamera* man = instance::cam_get_man(context);

	Vec2f center = instance::cam_get_pos(context);

	ImGui::Text("Center (%3.2f, %3.2f)", center.x, center.y);

	bool lock = instance::cam_get_lock_enabled(context);
	if (ImGui::Checkbox("Lock Position", &lock)) {
		instance::cam_set_lock_enabled(context, lock);
	}


	float pos[2] = { center.x, center.y };

	if (ImGui::DragFloat2("Set Pos", pos)) {
		instance::cam_set_lock_enabled(context, true);
		instance::cam_set_pos(context, Vec2f{ pos[0], pos[1] });
	}


	auto& targets = man->getTargets();

	ImGui::Separator();
	for (unsigned i = 0; i < targets.size(); i++) {
		auto& target = targets[i];

		/*
		if (target.type == GameCamera::TargetType::MOVING) {
			ImGui::Text("Target: Moving(%3.2f, %3.2f)", target.movingTarget->x, target.movingTarget->y);
		}
		else if (target.type == GameCamera::TargetType::STATIC) {
			ImGui::Text("Target: Static(%3.2f, %3.2f)", target.staticTarget.x, target.staticTarget.y);
		}
		else {
			ImGui::Text("Target: Invalid");
		}
		*/
		//ImGui::Text("Offset (%3.2f, %3.2f)", target.offset.x, target.offset.y);
		//ImGui::Text("Target Priority: %u", target.priority);


		ImGui::Text("Target Active: %s", 
			(target->get_state() == CamTargetState::Active ? "Active" : "Inactive")
		);
		Vec2f pos = target->get_target_pos();
		ImGui::Text("Target Position: (%3.2f, %3.2f)", pos.x, pos.y);

		const char* priority_str;
		switch (target->get_priority()) {
		case CamTargetPriority::Low:	priority_str = "Low";	 break;
		case CamTargetPriority::Medium: priority_str = "Medium"; break;
		case CamTargetPriority::High:	priority_str = "High";	 break;
		}
		ImGui::Text("Target Priority: %s", priority_str);


		ImGui::Separator();
	}
	float zoom = instance::cam_get_zoom(context);
	if (ImGui::SliderFloat("Zoom", &zoom, 0.25f, 4.f, "%.2f")) {
		instance::cam_set_zoom(context, roundf(zoom * 4.f) / 4.f);
	}


	ImGui::SameLine();
	if (ImGui::Button("Reset")) {
		instance::cam_set_zoom(context, 1.f);
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

		Instance(instanceID)->want_reset = true;
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
		for (auto& obj : instance::obj_get_man(context)->getObjects()) {
			if (obj->showInspect && ImGui::Begin(obj->getTypeName().c_str(), &obj->showInspect)) {
				obj->ImGui_Inspect();
				ImGui::End();
			}
		}
	}

}

}
