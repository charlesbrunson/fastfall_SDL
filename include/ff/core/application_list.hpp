#pragma once

#include <stack>
#include <memory>

#include "ff/gfx/color.hpp"
#include "ff/core/application.hpp"

namespace ff {

class application_list {
public:
    application_list();
    explicit application_list(std::unique_ptr<application>&& t_app);

	void update();
	bool empty() const;
	void clear();

    application* get_active_app() const;

protected:
	std::unique_ptr<application> m_root;
};

}