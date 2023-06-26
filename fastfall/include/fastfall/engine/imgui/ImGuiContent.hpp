#pragma once

#include "imgui.h"
#include "fastfall/engine/time/time.hpp"

#include <string>
#include <sstream>

namespace ff {

// where to display this content
enum class ImGuiContentType {
	NONE = -1,	// other content will include this one
	SIDEBAR_LEFT = 0,
	SIDEBAR_RIGHT = 1,
};
constexpr int ImGuiContentTypeCount = 3;

class ImGuiContent {
private:
	bool isOpen = true;
	bool isMenu = false;

	std::string contentTag;
	std::string name;
	std::string menuName;
	ImGuiContentType ImGui_Type;
	size_t IDnum;

	static size_t initCounter;

	bool contentAdded = false;

public:

	ImGuiContent(ImGuiContentType type, const std::string& title, const std::string& onMenuName = "");
	virtual ~ImGuiContent();

	inline ImGuiContentType ImGui_getType() const {
		return ImGui_Type;
	};
	inline bool& ImGui_getOpen() {
		return isOpen;
	};
	inline bool ImGui_hasMenu() {
		return isMenu;
	};
	inline const std::string& ImGui_ContentTag() {
		return contentTag;
	};
	inline const char* ImGui_MenuName() {
		return menuName.c_str();
	};
	inline const char* ImGui_Name() {
		return name.c_str();
	};
	inline void ImGui_setOpen(bool open = true) {
		isOpen = open;
	};


	void ImGui_addContent();

	virtual void ImGui_getContent(secs deltaTime) = 0;
	virtual void ImGui_getExtraContent() {};

};

}
