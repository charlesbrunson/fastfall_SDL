#pragma once

#include "ff/core/time.hpp"
#include "ff/core/event.hpp"
#include "ff/gfx/color.hpp"
#include "ff/gfx/view.hpp"
#include "ff/gfx/render_target.hpp"

#include <assert.h>
#include <type_traits>
#include <memory>
#include <string>
#include <string_view>
#include <optional>

namespace ff {

struct render_info {
    uvec2 size;
};

enum class application_action {
    // continue running this state
    Continue,

    // notify state handler of next_app, next_app becomes active
    Next,
    // exit and free this app, next_app becomes active
    ExitNext,

    // exit and free this app, revert to prev_app, closes game if last state
    ExitPrev,
    // exit and free all app, engine will finish running
    CloseAll,
};

class application {
    friend class application_list;

public:
    application(std::string_view t_app_name)
    : m_app_name(t_app_name)
    {}

    application(std::string_view t_app_name, color t_clear_color)
    : m_app_name(t_app_name)
    , m_clear_color(t_clear_color)
    {}

    application(const application&) = delete;
    application& operator=(const application&) = delete;
    application(application&&) = delete;
    application& operator=(application&&) = delete;
    virtual ~application() = default;

	virtual void update(seconds t_deltatime) = 0;
	virtual void predraw(tick_info t_tick, render_info t_rinfo) = 0;
    virtual void draw(const render_target& t_target) {};

    virtual bool push_event(const SDL_Event& t_event) { return false; };
    virtual void update_imgui() {};
    virtual std::optional<view> get_view() const noexcept { return {}; };

	[[nodiscard]] inline color get_clear_color() const noexcept { return m_clear_color; };
	[[nodiscard]] inline application* get_prev_app() const noexcept { return m_prev_app; };
	[[nodiscard]] inline application* get_next_app() const noexcept { return m_next_app.get(); };
	[[nodiscard]] inline application_action get_app_action() const noexcept { return m_action; };

	template<typename T, typename... Ts>
		requires std::is_base_of_v<application, std::decay_t<T>>
			&& (!std::is_same_v<application, std::decay_t<T>>)
	void create(Ts&&... params) {
		// protect states down chain from getting dropped
        auto tmp = std::move(m_next_app);
        m_next_app = std::make_unique<T>(std::forward<Ts>(params)...);
        m_next_app->m_next_app = std::move(tmp);
        m_next_app.get()->m_prev_app = this;
	}

protected:
    inline void set_app_action(application_action t_act) { m_action = t_act; };
	inline bool is_active() const { return m_active; };

    color m_clear_color;

private:
    bool m_active = false;
    std::string m_app_name;
	std::unique_ptr<application> m_next_app;
    application* m_prev_app = nullptr;
    application_action m_action = application_action::Continue;
};

}
