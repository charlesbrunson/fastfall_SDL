#include "fastfall/render/Window.hpp"
#include "fastfall/render/opengl.hpp"
#include "fastfall/render.hpp"

#include "detail/error.hpp"


namespace ff {

Window::Window()
	: RenderTarget(),
	m_window{ SDL_CreateWindow(
		"",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		480,
		360,
		SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN
	) }
{
	init();
	m_id = SDL_GetWindowID(m_window);
}

Window::Window(const char* title, unsigned initWidth, unsigned initHeight)
	: RenderTarget(),
	m_window{ SDL_CreateWindow(
		title,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		initWidth,
		initHeight,
		SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN
	) }
{	
	init();
	m_id = SDL_GetWindowID(m_window);
}


Window::Window(std::string_view title, unsigned initWidth, unsigned initHeight)
	: RenderTarget(),
	m_window{ SDL_CreateWindow(
		title.data(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		initWidth,
		initHeight,
		SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN
	)}
{
	init();
	m_id = SDL_GetWindowID(m_window);
}

void Window::init() {
	assert(ff::FFisInit());

	m_context = SDL_GL_CreateContext(m_window);
	checkSDL(m_context);

	ff::FFinitGLEW();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(m_window, m_context);
	glCheck(ImGui_ImplOpenGL3_Init("#version 330"));

	setDefaultView();
}

Window::~Window() 
{
	SDL_GL_DeleteContext(m_context);
	SDL_DestroyWindow(m_window);
}

SDL_Window* Window::getSDL_Window() const
{
	return m_window;
}

void Window::setWindowResizable(bool enable) {
	SDL_SetWindowResizable(m_window, enable ? SDL_TRUE : SDL_FALSE);
}
void Window::setWindowBorderless(bool enable) {
	SDL_SetWindowBordered(m_window, !enable ? SDL_TRUE : SDL_FALSE);
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
	checkSDL(SDL_GL_SetSwapInterval(enable ? 1 : 0));
}


void Window::setWindowTitle(std::string_view title) {
	SDL_SetWindowTitle(m_window, title.data());
}

void Window::setWindowFullscreen(FullscreenType set) {
	switch (set) {
	case FullscreenType::FULLSCREEN:
		SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
		break;
	case FullscreenType::FULLSCREEN_DESKTOP:
		SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		break;
	case FullscreenType::WINDOWED:
		SDL_SetWindowFullscreen(m_window, 0);
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
	SDL_GL_SwapWindow(m_window);
}

glm::ivec2 Window::getSize() const {
	glm::ivec2 size;
	SDL_GetWindowSize(m_window, &size.x, &size.y);
	return size;
}

glm::fvec2 Window::windowCoordToWorld(glm::ivec2 windowCoord) {
	return windowCoordToWorld(windowCoord.x, windowCoord.y);
}

glm::ivec2 Window::worldCoordToWindow(glm::fvec2 worldCoord) {
	return worldCoordToWindow(worldCoord.x, worldCoord.y);
}

glm::fvec2 Window::windowCoordToWorld(int windowCoordX, int windowCoordY) {

	
	glm::fvec3 ndc{
		(float)windowCoordX / (float)getSize().x,
		(float)windowCoordY / (float)getSize().y,
		1.f
	};
	ndc.x = (ndc.x * 2.f) - 1.f;
	ndc.y = (ndc.y * 2.f) - 1.f;

	glm::mat3 view_mat = getView().getInvMatrix();
	glm::fvec3 world_coord = view_mat * ndc;
	

	//auto viewport = getView().getViewport();



	//glm::fvec2 world_coord = getView().getInvMatrix() * ndc;
	return world_coord;
}

glm::ivec2 Window::worldCoordToWindow(float worldCoordX, float worldCoordY) {

	glm::mat3 view_mat = getView().getMatrix();
	glm::fvec3 ndc = view_mat * glm::fvec3{ worldCoordX, worldCoordY, 1.f };
	ndc.x = (ndc.x + 1.f) / 2.f;
	ndc.y = (ndc.y + 1.f) / 2.f;

	return glm::ivec2{ roundf(ndc.x * getSize().x), roundf(ndc.y * getSize().y) };
}

unsigned int Window::getID() {
	return m_id;
}

} //namespace ff
