#include "fastfall/engine/imgui/ImGuiFrame.hpp"
#include "fastfall/engine/config.hpp"

#include <assert.h>

#include "imgui.h"

#include "fastfall/util/log.hpp"
#include "fastfall/render/Color.hpp"

namespace ff {

ImGuiFrame ImGuiFrame::frame;

const std::string ImGuiFrame::leftPanel = "##leftpanel";
const std::string ImGuiFrame::rightPanel = "##rightpanel";
const std::string ImGuiFrame::logPanel = "##logpanel";

Color logHighlight[static_cast<unsigned int>(log::level::SIZE)] = {
	Color(128, 128, 128), // NONE
	Color(128, 128, 128), // STEP
	Color(128, 128, 128), // VERBOSE
	Color(255, 255, 255), // INFO
	Color(255, 255, 100), // WARN
	Color(255, 100, 100), // ERROR
};

ImGuiFrame::ImGuiFrame() {

};

ImGuiFrame& ImGuiFrame::getInstance() {
	return frame;
}

void ImGuiFrame::addContent(ImGuiContent* content) {
	imguiContent[static_cast<unsigned int>(content->ImGui_getType())].insert(content);
}
void ImGuiFrame::removeContent(ImGuiContent* content) {

	std::set<ImGuiContent*>& set = imguiContent[static_cast<unsigned int>(content->ImGui_getType())];

	if (!set.empty()) {
		set.erase(content);
	}
}
void ImGuiFrame::resize(Recti outer, Vec2u innersize) {
	int sideMargin = (outer.width - innersize.x) / 2;
	int topMargin = (outer.height - innersize.y) / 2;

	left_SideBarArea = Recti(0, 0, sideMargin, outer.height);
	right_SideBarArea = Recti(sideMargin + innersize.x, 0, sideMargin, outer.height);
	upper_consoleArea = Recti(sideMargin, 0, innersize.x, topMargin);
	lower_consoleArea = Recti(sideMargin, topMargin + innersize.y, innersize.x, topMargin);;
}

void ImGuiFrame::clear() {
	for (unsigned i = 0; i < ImGuiContentTypeCount; i++) {
		imguiContent[i].clear();
	}

}

void ImGuiFrame::displaySidePanel(std::set<ImGuiContent*>& contents, Recti area, const char* panelName) {

	ImGui::SetNextWindowSize(ImVec2(area.getSize().x, area.getSize().y), ImGuiCond_Always);
	ImGui::SetNextWindowPos(
		ImVec2(	ImGui::GetMainViewport()->Pos.x + area.getPosition().x, 
				ImGui::GetMainViewport()->Pos.y + area.getPosition().y), 
		ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.2f);
	ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

	if (ImGui::Begin(panelName, NULL,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		//ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		//ImGuiWindowFlags_NoDocking | 
		0
	)) {

		if (ImGui::BeginMenuBar())
		{
			for (auto& content : contents) {
				if (content->ImGui_hasMenu()) {
					if (ImGui::BeginMenu(content->ImGui_MenuName()))
					{
						ImGui::MenuItem(content->ImGui_Name(), NULL, &content->ImGui_getOpen());
						ImGui::EndMenu();
					}
				}
			}
			ImGui::EndMenuBar();
		}

		if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_AutoSelectNewTabs))
		{
			for (auto& content : contents) {
				if (content->ImGui_getOpen()) {
					if (ImGui::BeginTabItem(content->ImGui_Name(), &content->ImGui_getOpen(), ImGuiTabItemFlags_None)) {
						content->ImGui_getContent();

						ImGui::EndTabItem();
					}
					// show any additional windows content may have, as long as if tab exists
					content->ImGui_getExtraContent();
				}
			}
			ImGui::EndTabBar();
		}

	}
	ImGui::End();
}


void ImGuiFrame::displayLog(Recti area, const char* panelName) {

	ImGui::SetNextWindowSize(ImVec2(area.getSize().x, area.getSize().y), ImGuiCond_Always);
	ImGui::SetNextWindowPos(
		ImVec2(	ImGui::GetMainViewport()->Pos.x + area.getPosition().x,
				ImGui::GetMainViewport()->Pos.y + area.getPosition().y),
		ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.2f);
	ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
	if (ImGui::Begin(panelName, NULL,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoDocking
		| 0
	)) {

		static bool ScrollToBottom = true;
		static bool fizzbuzz = false;

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Log Menu")) {
				if (ImGui::BeginMenu("Set Verbosity")) {
					if (ImGui::MenuItem("Verbose", NULL)) {
						log::set_verbosity(log::level::VERB);
					}
					if (ImGui::MenuItem("Info", NULL)) {
						log::set_verbosity(log::level::INFO);
					}
					if (ImGui::MenuItem("Warn", NULL)) {
						log::set_verbosity(log::level::WARN);
					}
					ImGui::EndMenu();
				}
				ImGui::MenuItem("Autoscroll", NULL, &ScrollToBottom);

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));

		log::level verbosity = log::get_verbosity();
		for (auto& entry : log::get_entries()) {
			if (entry.lvl < verbosity)
				continue;


			if (logHighlight[static_cast<unsigned int>(entry.lvl)].hex() != 0xFFFFFFFF) {

				Color col = logHighlight[static_cast<unsigned int>(entry.lvl)];

				static float inv_255 = 1.f / 255.f;
				ImGui::TextColored(
					ImVec4(col.r * inv_255,
						col.g * inv_255,
						col.b * inv_255,
						col.a * inv_255),
					"%s", entry.message.c_str());

			}
			else {
				ImGui::Text("%s", entry.message.c_str());
			}
		}
		if (ScrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
			ImGui::SetScrollHereY(1.0f);
			//ScrollToBottom = false;
		}

		ImGui::PopStyleVar();
		ImGui::EndChild();

		ImGui::End();
	}
}

void ImGuiFrame::display() {
	if (!enabled)
		return;

	//LEFT PANEL
	displaySidePanel(imguiContent[static_cast<unsigned int>(ImGuiContentType::SIDEBAR_LEFT)], left_SideBarArea, ImGuiFrame::leftPanel.c_str());

	//RIGHT PANEL
	displaySidePanel(imguiContent[static_cast<unsigned int>(ImGuiContentType::SIDEBAR_RIGHT)], right_SideBarArea, ImGuiFrame::rightPanel.c_str());

	//TOP LOG
	displayLog(upper_consoleArea, ImGuiFrame::logPanel.c_str());

}

}