#pragma once

#include "imgui.h"

namespace ff {

bool imgui_init(void* window, void* gl_context, const char* glsl_version);
void imgui_new_frame();
void imgui_render();
void imgui_display();
void imgui_quit();

}