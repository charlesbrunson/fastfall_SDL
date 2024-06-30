#include "ff/gfx/window.hpp"

#include "../external/sdl.hpp"
#include "../external/glew.hpp"
#include "../external/imgui.hpp"

#include "ff/util/log.hpp"

#include <cassert>

namespace ff {

window::window(bool start_hidden)
	: // RenderTarget(),
	m_window_impl{ (void*)SDL_CreateWindow(
		"",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		480,
		360,
		SDL_WINDOW_OPENGL | (start_hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN) | SDL_WINDOW_RESIZABLE
	) }
{
	init();
}

window::window(std::string_view t_title, uvec2 t_size, bool t_start_hidden)
	: // RenderTarget(),
	m_window_impl{ SDL_CreateWindow(
		t_title.data(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		t_size.x,
		t_size.y,
		SDL_WINDOW_OPENGL | (t_start_hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN) | SDL_WINDOW_RESIZABLE
	) }
{	
	init();
}

window::window(window&& t_window) noexcept
: m_window_impl{t_window.m_window_impl}
, m_has_imgui{t_window.m_has_imgui}
, m_id{t_window.m_id}
{
    m_just_cleared = t_window.m_just_cleared;
    m_view = t_window.m_view;
    m_gl_context = t_window.m_gl_context;
    m_framebuffer = t_window.m_framebuffer;
    //m_prev_draw_call = t_window.m_prev_draw_call;

    t_window.m_window_impl = nullptr;
    t_window.m_has_imgui = false;
    t_window.m_id = 0;
    t_window.m_just_cleared = false;
    t_window.m_view = {};
    t_window.m_gl_context = nullptr;
    t_window.m_framebuffer = 0;
    //t_window.m_prev_draw_call = {};
}

window& window::operator=(window&& t_window) noexcept {
    m_window_impl = t_window.m_window_impl;
    m_has_imgui = t_window.m_has_imgui;
    m_id = t_window.m_id;
    m_just_cleared = t_window.m_just_cleared;
    m_view = t_window.m_view;
    m_gl_context = t_window.m_gl_context;
    m_framebuffer = t_window.m_framebuffer;
    //m_prev_draw_call = t_window.m_prev_draw_call;

    t_window.m_window_impl = nullptr;
    t_window.m_has_imgui = false;
    t_window.m_id = 0;
    t_window.m_just_cleared = false;
    t_window.m_view = {};
    t_window.m_gl_context = nullptr;
    t_window.m_framebuffer = 0;
    //t_window.m_prev_draw_call = {};
    return *this;
}

void window::init() {
    m_id = SDL_GetWindowID((SDL_Window*)m_window_impl);
    m_gl_context = SDL_GL_CreateContext((SDL_Window*)m_window_impl);
    checkSDL(m_gl_context);

	if (!ff::glew_init()) {
		SDL_DestroyWindow((SDL_Window*)m_window_impl);
		m_window_impl = nullptr;
        ff::error("{}", "glew failed to init");
		return;
	}

    SDL_GL_MakeCurrent((SDL_Window*)m_window_impl, m_gl_context);

#if defined(__EMSCRIPTEN__)
    m_has_imgui = imgui_init(window_impl, m_gl_context, "#version 300 es");
#else
    m_has_imgui = imgui_init(m_window_impl, m_gl_context, "#version 330");
#endif

	set_default_view();
}

window::~window()
{
    if (m_has_imgui) {
        imgui_quit();
    }
    if (m_gl_context) {
        SDL_GL_DeleteContext(m_gl_context);
    }
	if (m_window_impl) {
        SDL_DestroyWindow((SDL_Window*)m_window_impl);
    }
}

/*
SDL_window* window::getSDL_window() const
{
	return m_window;
}
*/

void window::set_resizeable(bool t_enable) {
#if not defined(__EMSCRIPTEN__)
	SDL_SetWindowResizable((SDL_Window*)m_window_impl, t_enable ? SDL_TRUE : SDL_FALSE);
#endif
}
void window::set_borderless(bool t_enable) {
#if not defined(__EMSCRIPTEN__)
	SDL_SetWindowBordered((SDL_Window*)m_window_impl, !t_enable ? SDL_TRUE : SDL_FALSE);
#endif
}

void window::set_size(uvec2 size) {
	assert(size.x > 0 && size.y > 0);
	SDL_SetWindowSize((SDL_Window*)m_window_impl, size.x, size.y);
}

void window::set_position(ivec2 pos) {
	SDL_SetWindowPosition((SDL_Window*)m_window_impl, pos.x, pos.y);
}

void window::set_centered() {
	SDL_SetWindowPosition((SDL_Window*)m_window_impl, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void window::set_vsync(bool enable)
{
	make_active();
	if (SDL_GL_SetSwapInterval(enable ? 1 : 0) != 0) {
		ff::warn("Vsync not supported");
	}
}


void window::set_title(std::string_view title) {
	SDL_SetWindowTitle((SDL_Window*)m_window_impl, title.data());
}

void window::set_display_mode(display_mode set) {
	switch (set) {
	case display_mode::Fullscreen:
#if not defined(__EMSCRIPTEN__)
		SDL_SetWindowFullscreen((SDL_Window*)m_window_impl, SDL_WINDOW_FULLSCREEN);
#else
		SDL_SetWindowFullscreen(m_window, SDL_bool::SDL_TRUE);
#endif
		break;
	case display_mode::Fullscreen_Desktop:
#if not defined(__EMSCRIPTEN__)
		SDL_SetWindowFullscreen((SDL_Window*)m_window_impl, SDL_WINDOW_FULLSCREEN_DESKTOP);
#else
		SDL_SetWindowFullscreen((SDL_Window*)window_impl, SDL_bool::SDL_TRUE);
#endif
		break;
	case display_mode::Windowed:
		SDL_SetWindowFullscreen((SDL_Window*)m_window_impl, SDL_bool::SDL_FALSE);
		break;
	}
}

void window::show(bool visible)
{
	if (visible) {
		SDL_ShowWindow((SDL_Window*)m_window_impl);
	}
	else {
		SDL_HideWindow((SDL_Window*)m_window_impl);
	}
}

void window::make_active()
{
	checkSDL(SDL_GL_MakeCurrent((SDL_Window*)m_window_impl, m_gl_context));
}


ivec2 window::position() {
	ivec2 r;
	SDL_GetWindowPosition((SDL_Window*)m_window_impl, &r.x, &r.y);
	return r;
}

void window::display() {
    //ZoneScoped;
    //TracyGpuZone("Display");
    // glFinish();
	SDL_GL_SwapWindow((SDL_Window*)m_window_impl);
    //TracyGpuCollect;
}

ivec2 window::size() const {
	ivec2 size;
	SDL_GetWindowSize((SDL_Window*)m_window_impl, &size.x, &size.y);
	return size;
}

unsigned window::id() const {
    return m_id;
}

/*
unsigned int window::getID() {
	return m_id;
}
*/

} //namespace ff
