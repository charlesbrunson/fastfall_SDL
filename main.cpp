#include "ff/engine.hpp"

#include "ff/core/loop.hpp"

#include "ff/gfx/vertex.hpp"

#include <thread>
#include <chrono>

class test_app : public ff::application {
public:
    test_app() : ff::application{ "test_app" } {};
    virtual void update(ff::seconds deltaTime) {};
    virtual void predraw(ff::tick_info predraw_state, const ff::window_info* win_info) {};
};

int main(int argc, char* argv[]) {
    ff::initialize();
    ff::loop{ std::make_unique<test_app>() }.run();
    ff::shutdown();
}