#include "ff/engine.hpp"

#include "ff/gfx/window.hpp"
#include "ff/core/application.hpp"
// #include "ff/core/loop.hpp"

#include "ff/gfx/vertex.hpp"

#include <thread>
#include <chrono>

class test_app : public ff::application {

};



int main(int argc, char* argv[]) {
    ff::initialize();


    auto vinfo = ff::get_vertex_attribute_info<ff::vertex>();

    {
        auto window = ff::window();
        window.show(true);

    }

    ff::shutdown();

}