#pragma once

#include <string_view>
#include <memory>

#include "util/math.hpp"

namespace ff {

// wrapper for a sdl opengl window

class window {
public:
	enum class mode {
		Fullscreen,
		Fullscreen_Desktop,
		Windowed
	};

	window(bool start_hidden = true);
	window(const char* title, unsigned initWidth, unsigned initHeight, bool start_hidden = true);
	window(std::string_view title, unsigned initWidth, unsigned initHeight, bool start_hidden = true);
	~window();

	// window controls
	void set_resizeable(bool enable = true);
	void set_borderless(bool enable = true);

	void set_size(const glm::uvec2& size);
	void set_size(unsigned W = 0, unsigned H = 0);

	void set_position(const glm::ivec2& pos);
	void set_position(int X, int Y);

	void set_centered();

	void set_mode(mode set);

	void set_title(std::string_view title);

	void set_vsync(bool enable = true);

	glm::ivec2 get_position();
	void show(bool visible = true);

	//SDL_Window* getSDL_Window() const;

	glm::ivec2 get_size() const;
	void set_active();

	void display();

	inline bool valid() { return window_impl != nullptr; };

private:
	void init();

    void* window_impl;
};

}