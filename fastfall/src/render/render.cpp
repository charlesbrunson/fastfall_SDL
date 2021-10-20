
#include "fastfall/render.hpp"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl.h"
#include "fastfall/render/opengl.hpp"
#include "fastfall/render/Window.hpp"

#include <stdexcept>
#include <string>

#include "detail/error.hpp"

#include "SDL_image.h"
#include "imgui.h"

#include <iostream>
#include <stack>
#include <mutex>

#if defined(__EMSCRIPTEN__)
#include "emscripten.h"
#endif

namespace ff {
namespace {
    bool renderInitialized = false;
    bool glewInitialized   = false;


    void GLAPIENTRY
        MessageCallback(GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length,
            const GLchar* message,
            const void* userParam)
    {

		if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
			fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
				(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
				type, severity, message);
		}
    }

}

bool FFinit()
{
    assert(!renderInitialized);
    renderInitialized = true;

    checkSDL(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER));

    SDL_version version;
    SDL_GetVersion(&version);
    LOG_INFO("{:>10} {}.{}.{}", "SDL", version.major, version.minor, version.patch);

    checkSDL(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1));
    //checkSDL(SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1));

    checkSDL(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0));

#if defined(__EMSCRIPTEN__)
    checkSDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES));
#else
    checkSDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3));
    checkSDL(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));
#endif

    int flags = IMG_INIT_PNG;
    int outflags = IMG_Init(flags);
    if (outflags != flags) {
        LOG_ERR_("IMG init failed: {}", IMG_GetError());
        renderInitialized = false;
    }

    return renderInitialized;
}

void FFquit()
{
    assert(renderInitialized);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

    IMG_Quit();
    SDL_Quit();
    renderInitialized = false;
}

bool FFisInit()
{
	return renderInitialized;
}

void FFinitGLEW() {
    if (glewInitialized) return;

    GLenum glew_err = glewInit();
    if (GLEW_OK != glew_err) {
        glewInitialized = false;
        std::string err = (char*)glewGetErrorString(glew_err);
        LOG_ERR_("Unable to init glew: {}", err);
        return;
    }
    glewInitialized = true;
    LOG_INFO("{:>10} {}", "GLEW", glewGetString(GLEW_VERSION));


    GLint glvmajor, glvminor;
    glGetIntegerv(GL_MAJOR_VERSION, &glvmajor);
    glGetIntegerv(GL_MINOR_VERSION, &glvminor);
#if defined(__EMSCRIPTEN__)
    LOG_INFO("{:>10} {}.{}", "OpenGL ES", glvmajor, glvminor);
#else
    LOG_INFO("{:>10} {}.{}", "OpenGL", glvmajor, glvminor);
#endif

    ShaderProgram::getDefaultProgram();
    LOG_INFO("Loaded default shader");

	ShaderProgram::getTileArrayProgram();
    LOG_INFO("Loaded tile array shader");

#if not defined(__EMSCRIPTEN__)
    glCheck(glEnable(GL_DEBUG_OUTPUT));
    glDebugMessageCallback(MessageCallback, 0);
#endif

    glCheck(glDisable(GL_CULL_FACE));
    glCheck(glDisable(GL_DEPTH_TEST));
    glCheck(glEnable(GL_BLEND));

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImPlot::CreateContext();

}

bool FFisGLEWInit() {
    return glewInitialized;
}

void ImGuiNewFrame(Window& window) {

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window.getSDL_Window());
    ImGui::NewFrame();
}

void ImGuiRender() {

    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }
}

namespace {

    std::mutex stale_lock;
    std::vector<GLuint> staleVAO;
    std::vector<GLuint> staleVBO;

}

void glStaleVertexArrays(size_t count, const GLuint* vao) {
    std::lock_guard<std::mutex> guard(stale_lock);

    std::vector<GLuint> in(vao, vao + count);
    staleVAO.insert(staleVAO.end(), in.begin(), in.end());

}

void glStaleVertexBuffers(size_t count, const GLuint* vbo) {
    std::lock_guard<std::mutex> guard(stale_lock);

    std::vector<GLuint> in(vbo, vbo + count);
    staleVBO.insert(staleVBO.end(), in.begin(), in.end());
}

void glStaleVertexArrays(const GLuint vao) {
    std::lock_guard<std::mutex> guard(stale_lock);
    staleVAO.push_back(vao);
}

void glStaleVertexBuffers(const GLuint vbo) {
    std::lock_guard<std::mutex> guard(stale_lock);
    staleVBO.push_back(vbo);
}

void glDeleteStale() {
    std::lock_guard<std::mutex> guard(stale_lock);

    if (!staleVAO.empty()) {
        glDeleteVertexArrays(staleVAO.size(), &staleVAO[0]);
        staleVAO.clear();
    }
    if (!staleVBO.empty()) {
        glDeleteBuffers(staleVBO.size(), &staleVBO[0]);
        staleVBO.clear();
    }
}



}

