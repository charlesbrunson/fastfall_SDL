#pragma once

#include "fastfall/engine/imgui/ImGuiContent.hpp"

namespace ff {

class InstanceObserver : public ImGuiContent {
public:
	InstanceObserver();

	void ImGui_getContent() override;

	void ImGui_getExtraContent() override;
};

}