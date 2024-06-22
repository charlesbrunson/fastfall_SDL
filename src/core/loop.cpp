#include "ff/core/loop.hpp"

#include "ff/util/log.hpp"

#include "../external/imgui.hpp"
#include <imgui_impl_sdl2.h>

namespace ff {


tick_info update_timer(clock<>& t_clock) {
    auto tick = t_clock.tick();
    return tick;
}

void update_apps(tick_info& t_tick, application_list& t_app_list, seconds update_time) {
    bool update = t_tick.update_count > 0;
    while (t_tick.update_count > 0) {
        t_app_list.get_active_app()->update(update_time);
        --t_tick.update_count;
    }
}

void predraw(tick_info& t_tick, application_list& t_app_list, window& t_window) {
    window_info win_info {
        .window_size = t_window.size()
    };
    t_app_list.get_active_app()->predraw(t_tick, win_info);
}

void draw(application_list& t_app_list, window& t_window) {
    if (application* app = t_app_list.get_active_app()) {
        t_window.clear(app->get_clear_color());
        // t_target.draw(*app);
    }
}

void update_view(application_list& t_app_list, window& t_window) {
    if (!t_app_list.empty()) {
        camera cam = t_window.get_view();
        application* app = t_app_list.get_active_app();
        auto cam_pos = app->get_view_pos();
        auto cam_zoom = app->get_view_zoom();

        cam.set_center(cam_pos);
        cam.set_size({800, 600});
        t_window.set_view(cam);
    }
}

void update_app_list(application_list& t_app_list) {
    t_app_list.update();
}

void display(window& t_window) {
    imgui_display();
    t_window.display();
}

void sleep(clock<>& t_clock) {
    t_clock.sleep();
}

bool process_events(clock<>& t_clock, application_list& t_app_list, window& t_window) {
    bool discardAllMousePresses = false;

    bool should_close = false;

    SDL_Event event;
    auto event_count = 0u;

    auto& imgui_io = ImGui::GetIO();

    auto is_mouse_event = [](unsigned type) {
        return type == SDL_MOUSEBUTTONDOWN
            || type == SDL_MOUSEBUTTONUP
            || type == SDL_MOUSEWHEEL;
    };

    auto is_key_event = [](unsigned type) {
        return type == SDL_KEYDOWN
            || type == SDL_KEYUP;
    };

    while (SDL_PollEvent(&event)) {
        event_count++;
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (imgui_io.WantCaptureMouse && is_mouse_event(event.type)) {
            continue;
        }
        if (imgui_io.WantCaptureKeyboard && is_key_event(event.type)) {
            continue;
        }
        if (discardAllMousePresses && (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)) {
            continue;
        }

        switch (event.type) {
        case SDL_QUIT:
            should_close = true;
            break;
        case SDL_WINDOWEVENT:
            if (event.window.windowID != t_window.id()) {
                break;
            }
            switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                should_close = true;
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                discardAllMousePresses = true;
                break;
            }
            break;
        }
    }
    return !should_close;
}

void update_imgui(window& t_win) {
    static bool show_demo = true;
    imgui_new_frame();
    ImGui::ShowDemoWindow(&show_demo);
    imgui_render();

}

// -----------------------------------------------------------------------------

bool loop::run_single_thread() {

    tick_info tick{};
    predraw(tick, m_app_list, m_window);
    update_view(m_app_list, m_window);
    draw(m_app_list, m_window);

    m_window.show(true);

    m_running = true;
    while (is_running() && !m_app_list.empty()) {
        tick = update_timer(m_clock);
        m_uptime += tick.deltatime;
        m_running &= process_events(m_clock, m_app_list, m_window);
        update_apps(tick, m_app_list, m_clock.upsDuration());
        predraw(tick, m_app_list, m_window);
        update_view(m_app_list, m_window);
        draw(m_app_list, m_window);
        update_imgui(m_window);
        update_app_list(m_app_list);
        display(m_window);
        sleep(m_clock);
    }

    m_window.show(false);
    m_running = false;
    return true;
}

bool loop::run_dual_thread() {

    return true;
}

bool loop::run_web() {
#if FF_HAS_EMSCRIPTEN



    return true;
#else
    return false;
#endif
}


loop::loop(std::unique_ptr<application>&& t_app, window&& t_window)
: m_window{ std::move(t_window) }
, m_app_list{ std::move(t_app) }
{
}

bool loop::run(loop_mode t_loop_mode) {


#if FF_HAS_EMSCRIPTEN
    ff::info("Engine loop config: emscripten");
    return run_web();
#else
    switch (t_loop_mode) {
        case loop_mode::SingleThread:
            ff::info("Engine loop config: single thread");
            return run_single_thread();
        case loop_mode::DualThread:
            ff::info("Engine loop config: double thread");
            return run_dual_thread();
        default:
            return false;
    }
#endif
}

}