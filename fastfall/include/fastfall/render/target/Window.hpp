#pragma once

#include <string_view>

#include "SDL3/SDL.h"

#include "glm/glm.hpp"

#include "RenderTarget.hpp"

namespace ff {

// wrapper for for a sdl opengl window

class Window : public RenderTarget {
public:

	enum class FullscreenType {
		FULLSCREEN,
		FULLSCREEN_DESKTOP,
		WINDOWED
	};

	Window(bool start_hidden = true);
	Window(const char* title, unsigned initWidth, unsigned initHeight, bool start_hidden = true);
	Window(std::string_view title, unsigned initWidth, unsigned initHeight, bool start_hidden = true);
	~Window();

	// window controls
	void setWindowResizable(bool enable = true);
	void setWindowBorderless(bool enable = true);

	void setWindowSize(const glm::uvec2& size);
	void setWindowSize(unsigned W = SDL_WINDOWPOS_UNDEFINED, unsigned H = SDL_WINDOWPOS_UNDEFINED);

	void setWindowPosition(const glm::ivec2& pos);
	void setWindowPosition(int X, int Y);

	void setWindowCentered();

	void setWindowFullscreen(FullscreenType set);

	void setWindowTitle(std::string_view title);

	void setVsyncEnabled(bool enable = true);

	glm::ivec2 getPosition();
	void showWindow(bool visible = true);

	SDL_Window* getSDL_Window() const;

	glm::ivec2 getSize() const override;
	void setActive();

	void display();

	unsigned int getID();

	inline bool valid() { return m_window != nullptr; };

private:

	void init();

	SDL_Window* m_window;
	unsigned int m_id = 0;

};

}