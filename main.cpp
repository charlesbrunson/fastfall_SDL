
#include <imgui.h>

#include "src/external/glew.hpp"

#include "ff/engine.hpp"
#include "ff/core/loop.hpp"

#include "ff/gfx/shader.hpp"
#include "ff/gfx/gpu_buffer.hpp"
#include "ff/gfx/vertex_array.hpp"

#include "ff/util/log.hpp"

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
    ff::fvec3 pos;
    ff::color col;
    using normalized = std::index_sequence<1>;
};

vertex vertices[] = {
    { { -0.5f, -0.5f, 0.0f }, ff::color::blue },   // bot left
    { {  0.5f, -0.5f, 0.0f }, ff::color::red },  // bot right
    { { -0.5f,  0.5f, 0.0f }, ff::color::green }, // top left
    { {  0.5f,  0.5f, 0.0f }, ff::color::white }   // top right
};

ff::u32 indices[] = {
    0, 1, 2,
    3, 2, 1
};

class test_app : public ff::application {
public:
    test_app()
    : ff::application{ "test_app" }
    , buf{ sizeof(vertices) + sizeof(indices) }
    , varr{}
    , test_shader{ vert_src, frag_src }
    {
        m_clear_color = ff::color::from_floats(0.2f, 0.3f, 0.3f, 1.0f);

        auto verts = buf.subspan<vertex>(0, 3);
        verts.copy(vertices);
        auto ndxs  = buf.subspan<ff::u32>(sizeof(vertices), 6 );
        ndxs.copy(indices);

        varr.assign_vertex_buffer(0, verts);
        varr.assign_element_buffer(ndxs);
    };

    void update(ff::seconds deltaTime) override {
        time_prev = time_next;
        time_next += deltaTime;
    };

    void predraw(ff::tick_info predraw_state, ff::render_info win_info) override {
        time = std::lerp(time_prev, time_next, predraw_state.interp);
    };

    void draw(const ff::render_target& t_target) override {
        using namespace glm;
        auto size = fvec2{ t_target.size() };
        glViewport(0, 0, t_target.size().x, t_target.size().y);

        test_shader.bind();
        test_shader.set("uView",  mat4{ 1 });
        test_shader.set("uModel", rotate(scale(mat4{ 1 }, fvec3{16, 16, 1}), (float)time, vec3(0.f, 1.f, 0.f)));

        // test_shader.set("uProj",  glm::ortho(-1.f, 1.f, -10.f, 10.f));
        auto ratio = size.x / size.y;
        test_shader.set("uProj",  ortho(-180.f * ratio, 180.f * ratio, -120.f, 120.f));

        varr.bind();
        glDrawElements(GL_TRIANGLES, 6, varr.elements()->index_type, varr.elements()->offset);
    };

    void update_imgui() override {
        if (show_demo) ImGui::ShowDemoWindow(&show_demo);
    };

private:
    bool show_demo = true;

    ff::seconds time_prev = 0.f;
    ff::seconds time_next = 0.f;
    ff::seconds time      = 0.f;

    ff::shader test_shader;
    ff::gpu_buffer<> buf;
    ff::vertex_array varr;
};

int main(int argc, char* argv[]) {
    ff::engine engine;
    ff::window window{ "ffengine", { 360, 240 } };
    ff::loop loop{ window };
    loop.set_app<test_app>();
    loop.run(ff::loop_mode::SingleThread);
}