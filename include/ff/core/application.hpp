#pragma once

#include <assert.h>
#include <type_traits>
#include <memory>
#include <string>
#include <string_view>

#include "ff/core/time.hpp"
#include "ff/core/event.hpp"
#include "ff/util/math.hpp"
#include "ff/gfx/color.hpp"

namespace ff {

struct window_info {
    int scale;
    vec2u window_size;
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

    application(std::string_view app_name);
    application(const application&) = delete;
    application& operator=(const application&) = delete;
    application(application&&) = delete;
    application& operator=(application&&) = delete;
    virtual ~application() = default;

	virtual void update(seconds deltaTime) = 0;
	virtual void predraw(tick_info predraw_state, const window_info* win_info) = 0;

    virtual bool push_event(const SDL_Event& event) { return false; };

	inline vec2f get_view_pos() const noexcept { return viewPos; };
	inline float get_view_zoom() const noexcept { return viewZoom; };

	inline const color& get_clear_color() const noexcept { return clear_color; };

	inline application* get_prev_app() const noexcept { return prev_app; };
	inline application* get_next_app() const noexcept { return next_app.get(); };

	inline const application_action& get_app_action() noexcept { return action; };

	template<typename T, typename... Ts>
		requires std::is_base_of_v<application, std::decay_t<T>>
			&& (!std::is_same_v<application, std::decay_t<T>>)
	void create(Ts&&... params) {
		// protect states down chain from getting dropped
        auto tmp = std::move(next_app);
        next_app = std::make_unique<T>(std::forward<Ts>(params)...);
        next_app->next_app = std::move(tmp);
        next_app.get()->prev_app = this;
	}

protected:
	color clear_color;

	vec2f viewPos;
	float viewZoom = 1.f;

    inline void get_app_action(application_action n_act) { action = n_act; };
	inline bool is_active() { return active; };

private:
    std::string application_name = "";

	std::unique_ptr<application> next_app;
    application* prev_app = nullptr;

    application_action action = application_action::Continue;
	bool active = false;

	int stateIGID;
	static int stateIGIDCounter;
};

}
