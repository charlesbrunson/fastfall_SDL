
#include <imgui.h>

#include "src/external/glew.hpp"

#include "ff/engine.hpp"
#include "ff/core/loop.hpp"
#include "ff/gfx/shader.hpp"
#include "ff/gfx/vertex_array.hpp"
#include "ff/gfx/vertex_buffer.hpp"

constexpr std::string_view vert_src = R"(
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

uniform mat4 uView;
uniform mat4 uModel;
uniform mat4 uProj;

out vec4 vColor;

void main()
{
  gl_Position = uProj * uView * uModel * vec4(aPos, 1.0f);
  vColor = aColor;
}
)";

constexpr std::string_view frag_src = R"(
#version 330 core

in vec4 vColor;
out vec4 fColor;

void main()
{
  fColor = vColor;
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
    { { -0.5f, -0.5f, 0.0f }, ff::color::red }, // left
    { {  0.5f, -0.5f, 0.0f }, ff::color::green }, // right
    { {  0.0f,  0.5f, 0.0f }, ff::color::blue }  // top
};

class test_app : public ff::application {
public:
    test_app()
    : ff::application{ "test_app" }
    , vbuf{ vertices }
    , varr{ vbuf }
    {
        test_shader = *ff::shader_factory{}
            .add_vertex("vert.glsl", vert_src)
            .add_fragment("frag.glsl", frag_src)
            .build();

        m_clear_color = ff::color::from_floats(0.2f, 0.3f, 0.3f, 1.0f);
    };

    void update(ff::seconds deltaTime) override {
        time_prev = time_next;
        time_next += deltaTime;
    };

    void predraw(ff::tick_info predraw_state, ff::render_info win_info) override {
        time = std::lerp(time_prev, time_next, predraw_state.interp);
    };

    void draw(const ff::render_target& t_target) override {

        glViewport(0, 0, t_target.size().x, t_target.size().y);

        test_shader.bind();
        test_shader.set("uView",  glm::mat4{ 1 });
        test_shader.set("uModel", glm::rotate(glm::mat4{ 1 }, (float)time, glm::vec3(0.f, 1.f, 0.f)));
        test_shader.set("uProj",  glm::ortho(-1.f, 1.f, -1.f, 1.f));

        varr.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
    };

    void update_imgui() override {
        if (show_demo) ImGui::ShowDemoWindow(&show_demo);
    };

private:
    bool show_demo = true;

    ff::seconds time_prev = 0.f;
    ff::seconds time_next = 0.f;
    ff::seconds time = 0.f;


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