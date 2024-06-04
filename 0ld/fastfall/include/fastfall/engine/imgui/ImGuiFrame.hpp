#pragma once


#include "fastfall/util/math.hpp"

#include "ImGuiContent.hpp"
#include "fastfall/engine/time/time.hpp"


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
	void display(secs deltaTime);

	inline bool isDisplay() const noexcept {
		return enabled;
	}

	inline void setDisplay(bool is_enabled = true) noexcept {
		enabled = is_enabled;
	}

	void clear();

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

	std::vector<ImGuiContent*> imguiContent[ImGuiContentTypeCount];

	void displaySidePanel(secs deltaTime, std::vector<ImGuiContent*>& contents, Recti area, const char* panelName);
	void displayLog(Recti area, const char* panelName);

	static ImGuiFrame frame;
};

}