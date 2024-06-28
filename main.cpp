#include "ff/engine.hpp"

#include "ff/core/loop.hpp"

#include "imgui.h"

#include <thread>

#include "ff/gfx/shader.hpp"


constexpr std::string_view vert_src = R"(
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 ourColor;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
  gl_Position = projection * view * model * vec4(aPos, 1.0f);
  ourColor = aColor;
}
)";


constexpr std::string_view frag_src = R"(
#version 330 core

in vec3 ourColor;
out vec3 outputColor;

void main()
{
  outputColor = ourColor;
}
)";


class test_app : public ff::application {
public:
    test_app() : ff::application{ "test_app" } {

        test_shader = *ff::shader_factory{}
            .add_vertex("vert.glsl", vert_src)
            .add_fragment("frag.glsl", frag_src)
            .build();

    };
    void update(ff::seconds deltaTime) override {

    };

    void predraw(ff::tick_info predraw_state, ff::window_info win_info) override {

    };

    void update_imgui() override {
        if (show_demo) ImGui::ShowDemoWindow(&show_demo);
    };

private:
    bool show_demo = true;

    ff::shader test_shader;

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