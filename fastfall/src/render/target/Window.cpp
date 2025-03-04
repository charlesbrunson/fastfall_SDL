#include "fastfall/render/target/Window.hpp"
#include "fastfall/render/external/opengl.hpp"
#include "fastfall/render/render.hpp"

#include "../detail/error.hpp"

#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

namespace ff {

Window::Window(bool start_hidden)
	: RenderTarget(),
	m_window{ SDL_CreateWindow(
		"",
		480,
		360,
		SDL_WINDOW_OPENGL | (start_hidden ? SDL_WINDOW_HIDDEN : 0UL) | SDL_WINDOW_RESIZABLE
	) }
{
	init();
	if (m_window)
		m_id = SDL_GetWindowID(m_window);
}

Window::Window(const char* title, unsigned initWidth, unsigned initHeight, bool start_hidden)
	: RenderTarget(),
	m_window{ SDL_CreateWindow(
		title,
		initWidth,
		initHeight,
		SDL_WINDOW_OPENGL | (start_hidden ? SDL_WINDOW_HIDDEN : 0UL) | SDL_WINDOW_RESIZABLE
	) }
{
	init();
	if (m_window)
		m_id = SDL_GetWindowID(m_window);
}


Window::Window(std::string_view title, unsigned initWidth, unsigned initHeight, bool start_hidden)
	: RenderTarget(),
	m_window{ SDL_CreateWindow(
		title.data(),
		initWidth,
		initHeight,
		SDL_WINDOW_OPENGL | (start_hidden ? SDL_WINDOW_HIDDEN : 0UL) | SDL_WINDOW_RESIZABLE
	)}
{
	init();

	if (m_window)
		m_id = SDL_GetWindowID(m_window);
}

void Window::init() {
	assert(ff::render::is_init());

	SDL_SetWindowFullscreenMode(m_window, nullptr);

    m_context = SDL_GL_CreateContext(m_window);
    checkSDL(m_context);

	if (!ff::render::glew_init()) {
		SDL_DestroyWindow(m_window);
		m_window = nullptr;
		return;
	}

    TracyGpuContext;

	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForOpenGL(m_window, m_context);

#if defined(__EMSCRIPTEN__)
	glCheck(ImGui_ImplOpenGL3_Init("#version 300 es"));
#else
	glCheck(ImGui_ImplOpenGL3_Init("#version 330"));
#endif

	setDefaultView();
}

Window::~Window() 
{
	SDL_GL_DestroyContext(m_context);

	if (m_window)
		SDL_DestroyWindow(m_window);
}

SDL_Window* Window::getSDL_Window() const
{
	return m_window;
}

void Window::setWindowResizable(bool enable) {
#if not defined(__EMSCRIPTEN__)
	SDL_SetWindowResizable(m_window, enable);
#endif
}
void Window::setWindowBorderless(bool enable) {
#if not defined(__EMSCRIPTEN__)
	SDL_SetWindowBordered(m_window, !enable);
#endif
}

void Window::setWindowSize(const glm::uvec2& size) {
	assert(size.x > 0 && size.y > 0);
	SDL_SetWindowSize(m_window, size.x, size.y);
}
void Window::setWindowSize(unsigned W, unsigned H) {
	assert(W > 0 && H > 0);
	SDL_SetWindowSize(m_window, W, H);
}

void Window::setWindowPosition(const glm::ivec2& pos) {
	SDL_SetWindowPosition(m_window, pos.x, pos.y);
}
void Window::setWindowPosition(int X, int Y) {
	SDL_SetWindowPosition(m_window, X, Y);
}

void Window::setWindowCentered() {
	SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void Window::setVsyncEnabled(bool enable)
{
	setActive();
	if (!SDL_GL_SetSwapInterval(enable ? 1 : 0)) {
		LOG_WARN("Vsync not supported");
	}
}


void Window::setWindowTitle(std::string_view title) {
	SDL_SetWindowTitle(m_window, title.data());
}

void Window::setWindowFullscreen(FullscreenType set) {
	switch (set) {
	case FullscreenType::FULLSCREEN:
	case FullscreenType::FULLSCREEN_DESKTOP:
		SDL_SetWindowFullscreen(m_window, true);
		break;
	case FullscreenType::WINDOWED:
		SDL_SetWindowFullscreen(m_window, false);
		break;
	}
}

void Window::showWindow(bool visible)
{
	if (visible) {
		SDL_ShowWindow(m_window);
	}
	else {
		SDL_HideWindow(m_window);
	}
}

void Window::setActive()
{
	checkSDL(SDL_GL_MakeCurrent(m_window, m_context));
}


glm::ivec2 Window::getPosition() {
	glm::ivec2 r;
	SDL_GetWindowPosition(m_window, &r.x, &r.y);
	return r;
}

void Window::display() {
    ZoneScoped;
    TracyGpuZone("Display");
	glFinish();
	SDL_GL_SwapWindow(m_window);
    TracyGpuCollect;
}

glm::ivec2 Window::getSize() const {
	glm::ivec2 size;
	SDL_GetWindowSize(m_window, &size.x, &size.y);
	return size;
}

unsigned int Window::getID() {
	return m_id;
}

} //namespace ff
