
#include "imgui.h"

#include "fastfall/engine/imgui/ImGuiContent.hpp"
#include "fastfall/engine/imgui/ImGuiFrame.hpp"


namespace ff {

size_t ImGuiContent::initCounter = 0;

ImGuiContent::ImGuiContent(ImGuiContentType type, const std::string& title, const std::string& onMenuName) :
	ImGui_Type(type),
	IDnum(initCounter++),
	name(title),
	menuName(onMenuName)
{
	isMenu = !menuName.empty();
    contentTag = fmt::format("##{}", title);
}

void ImGuiContent::ImGui_addContent() {
	if (!contentAdded && ImGui_Type != ImGuiContentType::NONE) {
		contentAdded = true;
		ImGuiFrame::getInstance().addContent(this);
	}
};

ImGuiContent::~ImGuiContent() {
	if (ImGui_Type != ImGuiContentType::NONE)
		ImGuiFrame::getInstance().removeContent(this);
}

}