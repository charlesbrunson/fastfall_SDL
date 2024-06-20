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

    void run(loop_mode t_loop_mode = loop_mode::DualThread);

    [[nodiscard]] inline bool is_running() const { return m_running; };

private:
    bool m_running = false;
    window m_window;
    application_list m_app_list;
    clock<> m_clock;
};

}
