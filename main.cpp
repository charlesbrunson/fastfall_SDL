#include "ff/engine.hpp"

#include "ff/core/loop.hpp"

#include "imgui.h"

#include <thread>

class test_app : public ff::application {
public:
    test_app() : ff::application{ "test_app" } {};
    void update(ff::seconds deltaTime) override {};
    void predraw(ff::tick_info predraw_state, ff::window_info win_info) override {};
    void update_imgui() override { if (show_demo) ImGui::ShowDemoWindow(&show_demo); };

private:
    bool show_demo = true;

};

int main(int argc, char* argv[]) {
    ff::initialize();
    ff::loop loop{
        std::make_unique<test_app>(),
        ff::window{ "ffengine", { 800, 600 } }
    };
    loop.run(ff::loop_mode::DualThread);
    ff::shutdown();
}