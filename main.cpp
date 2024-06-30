
#include <imgui.h>

#include "src/external/glew.hpp"

#include "ff/engine.hpp"
#include "ff/core/loop.hpp"
#include "ff/gfx/shader.hpp"
#include "ff/gfx/vertex_array.hpp"
#include "ff/gfx/vertex_buffer.hpp"

constexpr std::string_view vert_src = R"(
#version 330 core

layout (location = 0) in vec3 t_pos;
layout (location = 1) in vec4 t_color;

out vec4 m_color;

void main()
{
  gl_Position = vec4(t_pos, 1.0f);
  m_color = t_color;
}
)";

constexpr std::string_view frag_src = R"(
#version 330 core

in vec4 m_color;
out vec4 outputColor;

void main()
{
  outputColor = m_color;
}
)";

struct vertex {
    ff::vec3 pos;
    ff::color col;
    using attributes = ff::v_attr_list<
        ff::v_attr<3, float>,
        ff::v_attr<4, ff::u8, true>
    >;
};

vertex vertices[] = {
    { { -0.5f, -0.5f, 1.0f }, ff::color::red }, // left
    { {  0.5f, -0.5f, 1.0f }, ff::color::red }, // right
    { {  0.0f,  0.5f, 1.0f }, ff::color::red }  // top
};

class test_app : public ff::application {
public:
    test_app()
    : ff::application{ "test_app" }
    , varr{ vbuf }
    , vbuf{ vertices }
    {
        test_shader = *ff::shader_factory{}
            .add_vertex("vert.glsl", vert_src)
            .add_fragment("frag.glsl", frag_src)
            .build();

        m_clear_color = ff::color::from_floats(0.2f, 0.3f, 0.3f, 1.0f);
    };

    void update(ff::seconds deltaTime) override {

    };

    void predraw(ff::tick_info predraw_state, ff::render_info win_info) override {

    };

    void draw(const ff::render_target& t_target) override {

        test_shader.bind();
        varr.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
    };

    void update_imgui() override {
        if (show_demo) ImGui::ShowDemoWindow(&show_demo);
    };

private:
    bool show_demo = true;

    ff::shader test_shader;
    ff::vertex_buffer vbuf;
    ff::vertex_array<vertex> varr;

};

int main(int argc, char* argv[]) {
    ff::initialize();
    {
        ff::loop loop{
            std::make_unique<test_app>(),
            ff::window{"ffengine", {800, 600}}
        };
        loop.run(ff::loop_mode::SingleThread);
    }
    ff::shutdown();
}