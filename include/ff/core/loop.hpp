#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include <functional>
#include <chrono>

#include <mutex>
#include <barrier>

#include <concepts>

#include "ff/core/application.hpp"
#include "ff/core/application_list.hpp"
#include "ff/gfx/window.hpp"

namespace ff {

class loop {
public:
    explicit loop(std::unique_ptr<application>&& t_app, window&& t_window = window{})
    : m_window{ std::move(t_window) }
    , m_app_list{ std::move(t_app) }
    {
    }

    void run() {

    }

private:
    window m_window;
    application_list m_app_list;
};

}
