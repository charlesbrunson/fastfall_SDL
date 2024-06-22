#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include <functional>
#include <chrono>

#include <mutex>
#include <barrier>

#include <concepts>

#include "ff/core/clock.hpp"
#include "ff/core/application.hpp"
#include "ff/core/application_list.hpp"
#include "ff/gfx/window.hpp"

namespace ff {

enum class loop_mode {
    SingleThread,
    DualThread,
};

class loop {
public:
    explicit loop(std::unique_ptr<application>&& t_app, window&& t_window = window{});

    bool run(loop_mode t_loop_mode);
    inline void stop() { m_running = false; }

    [[nodiscard]] inline bool is_running() const { return m_running; };

    window& get_window() { return m_window; }
    const window& get_window() const { return m_window; }

    clock<>& get_clock() { return m_clock; }
    const clock<>& get_clock() const { return m_clock; }

private:

    bool run_single_thread();
    bool run_dual_thread();
    bool run_web();

    // bool m_interpolate  = true;
    // bool m_pause_update = false;
    // bool m_step_update  = false;

    bool m_running = false;
    seconds m_uptime = 0.0;

    clock<> m_clock;
    application_list m_app_list;
    window m_window;
};

}
