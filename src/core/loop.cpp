#include "ff/core/loop.hpp"

namespace ff {

loop::loop(std::unique_ptr<application>&& t_app, window&& t_window)
: m_window{ std::move(t_window) }
, m_app_list{ std::move(t_app) }
{
}

void loop::run(loop_mode t_loop_mode) {

}

}