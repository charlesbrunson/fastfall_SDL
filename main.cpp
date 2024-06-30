
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

using namespace ff;

class test_app : public application {
public:
    test_app()
    : application{ "test_app" }
    , vbuf{ vertices }
    , varr{ vbuf }
    {
        test_shader = *shader_factory{}
            .add_vertex("vert.glsl", vert_src)
            .add_fragment("frag.glsl", frag_src)
            .build();

        m_clear_color = color::from_floats(0.2f, 0.3f, 0.3f, 1.0f);
    };

    void update(seconds deltatime) override {

    };

    void predraw(tick_info predraw_state, render_info win_info) override {
        if (predraw_state.update_count) {
            time_prev = time_next;
            time_next += predraw_state.deltatime;
        }
        time = std::lerp(time_prev, time_next, predraw_state.interp);
    };

    void draw(const render_target& t_target) override {

        auto size = t_target.size();
        glViewport(0, 0, size.x, size.y);

        fvec2 sizef = size;

        auto model = mat4{ 1.f };
        model = translate(model, vec3{ 100, 200, 0 });
        model = rotate(   model, (float)time * 2.f, vec3{ 0.f, 1.f, 0.f });
        model = scale(    model, vec3{ 100, -100, 0 });

        mat4 view = lookAt(
                vec3{0.f, 0.f, -1.f},
                vec3{0.f, 0.f, 0.f},
                vec3{0.f, -1.f, 0.f});

        mat4 proj = ortho(
                -0.5f * sizef.x, 0.5f * sizef.x, -0.5f * sizef.y, 0.5f * sizef.y,
                -1000.f, 1000.f);

        test_shader.bind();
        test_shader.set("uView",  view);
        test_shader.set("uModel", model);
        test_shader.set("uProj",  proj);

        varr.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
    };

    void update_imgui() override {
        if (show_demo) ImGui::ShowDemoWindow(&show_demo);
    };

private:
    bool show_demo = true;

    seconds time_prev = 0.f;
    seconds time_next = 0.f;
    seconds time = 0.f;

    shader test_shader;
    vertex_buffer vbuf;
    vertex_array<vertex> varr;
};

int main(int argc, char* argv[]) {
    engine engine;
    window window{ "ffengine", { 800, 600 } };
    loop loop{ window };
    loop.set_app<test_app>();
    loop.run(loop_mode::DualThread);
}