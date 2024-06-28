#pragma once

#include <string_view>
#include <memory>

#include "ff/util/math.hpp"
#include "render_target.hpp"

namespace ff {

// wrapper for a sdl opengl window
enum class display_mode {
    Fullscreen,
    Fullscreen_Desktop,
    Windowed
};

class window : public render_target {
public:
	window(bool t_start_hidden = true);
	window(std::string_view t_title, uvec2 t_size, bool t_start_hidden = true);
    window(const window&) = delete;
    window& operator=(const window&) = delete;
    window(window&&) noexcept;
    window& operator=(window&&) noexcept;
	~window() override;

	void set_resizeable(bool t_enable = true);
	void set_borderless(bool t_enable = true);
    void set_vsync(bool t_enable = true);
    void show(bool t_visible = true);

	void set_size(uvec2 t_size);
	void set_position(ivec2 t_pos);
	void set_centered();
	void set_display_mode(display_mode t_set);
	void set_title(std::string_view title);

    [[nodiscard]] ivec2 position();
    [[nodiscard]] ivec2 size() const override;
    [[nodiscard]] unsigned id() const;

	void make_active();
	void display();

	inline bool valid() { return m_window_impl != nullptr; };

private:
	void init();

    void* m_window_impl = nullptr;
    unsigned m_id = 0;
    bool m_has_imgui = false;
};

}