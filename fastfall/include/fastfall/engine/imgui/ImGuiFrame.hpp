#pragma once


#include "fastfall/util/math.hpp"

#include "ImGuiContent.hpp"


#include <set>
#include <map>
#include <memory>

namespace ff {

class ImGuiFrame {

public:
	//static bool create(Recti outer, Recti inner);
	static ImGuiFrame& getInstance();

	void addContent(ImGuiContent* content);
	void removeContent(ImGuiContent* content);
	void resize(Recti outer, Vec2u innersize);
	void display();

	inline bool isDisplay() const noexcept {
		return enabled;
	}

	inline void setDisplay(bool is_enabled = true) noexcept {
		enabled = is_enabled;
	}


	const static std::string leftPanel;
	const static std::string rightPanel;
	const static std::string logPanel;

	Recti left_SideBarArea;
	Recti right_SideBarArea;
	Recti upper_consoleArea;
	Recti lower_consoleArea;

private:
	bool enabled = false;

	ImGuiFrame();

	std::set<ImGuiContent*> imguiContent[ImGuiContentTypeCount];

	void displaySidePanel(std::set<ImGuiContent*>& contents, Recti area, const char* panelName);
	void displayLog(Recti area, const char* panelName);

	static ImGuiFrame frame;
};

}