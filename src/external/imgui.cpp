#include "imgui.hpp"

#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

namespace ff {

bool imgui_is_init = false;

bool imgui_init(void* window, void* gl_context, const char* glsl_version) {
    if (imgui_is_init)
        return true;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    auto sdl2_r = ImGui_ImplSDL2_InitForOpenGL((SDL_Window*)window, gl_context);
    auto gl_r   = ImGui_ImplOpenGL3_Init(glsl_version);
    imgui_is_init = sdl2_r && gl_r;
    return imgui_is_init;
}

void imgui_new_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void imgui_quit() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

}