#pragma once

#include "application_list.hpp"

namespace ff {

class application_runner {
public:
    application_runner(std::unique_ptr<application> init_app);

    inline bool is_running() const { return !app_list.empty(); }

private:
    application_list app_list;
};

}