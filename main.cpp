#include "ff/engine.hpp"

#include "ff/gfx/window.hpp"

#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    ff::initialize();

    auto window = ff::window();
    window.show(true);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    ff::shutdown();

}