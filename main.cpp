#include "ff/engine.hpp"

#include "ff/gfx/window.hpp"
#include "ff/core/application.hpp"
#include "ff/core/loop.hpp"

#include <thread>
#include <chrono>

class test_app : public ff::application {

};



int main(int argc, char* argv[]) {
    ff::initialize();

    {
        auto window = ff::window();
        window.show(true);

        ff::loop::run_with_app<test_app>();
    }

    ff::shutdown();

}