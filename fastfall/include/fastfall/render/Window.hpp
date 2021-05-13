#pragma once

#include <string_view>

#include <SDL.h>

#include <glm/glm.hpp>

#include "RenderTarget.hpp"

namespace ff {

// wrapper for for a sdl opengl window

class Window : public RenderTarget {
public:

	Window(const char* title, unsigned initWidth, unsigned initHeight);
	Window(std::string_view title, unsigned initWidth, unsigned initHeight);
	~Window();

	// window controls
	void setWindowResizable(bool enable = true);
	void setWindowBorderless(bool enable = true);

	void setWindowSize(const glm::uvec2& size);
	void setWindowSize(unsigned W = SDL_WINDOWPOS_UNDEFINED, unsigned H = SDL_WINDOWPOS_UNDEFINED);

	void setWindowPosition(const glm::ivec2& pos);
	void setWindowPosition(int X, int Y);

	void setWindowCentered();

	void setVsyncEnabled(bool enable = true);

	glm::ivec2 getPosition();



	void showWindow(bool visible = true);

	SDL_Window* getSDL_Window() const;

	glm::ivec2 getSize() const override;
	void setActive();

	void display();

	glm::fvec2 windowCoordToWorld(int windowCoordX, int windowCoordY);
	glm::fvec2 windowCoordToWorld(glm::ivec2 windowCoord);
	glm::ivec2 worldCoordToWindow(float worldCoordX, float worldCoordY);
	glm::ivec2 worldCoordToWindow(glm::fvec2 worldCoord);

	unsigned int getID();

private:

	void init();

	SDL_Window* m_window;
	unsigned int m_id = 0;

};

}