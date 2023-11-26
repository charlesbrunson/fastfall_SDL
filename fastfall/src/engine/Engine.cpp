
#include <queue>
#include <thread>

#include "fastfall/engine/imgui/ImGuiFrame.hpp"

#include "fastfall/util/log.hpp"

#include "fastfall/render/render.hpp"
#include "../render/detail/error.hpp"

#include "fastfall/engine/Engine.hpp"
#include "fastfall/engine/config.hpp"
#include "fastfall/engine/input/Mouse.hpp"

#include "fastfall/engine/input/InputConfig.hpp"
#include "fastfall/engine/time/profiler.hpp"

#include "fastfall/resource/ResourceWatcher.hpp"

#include "fastfall/resource/Resources.hpp"
#include "fastfall/render/drawable/AnimatedSprite.hpp"
#include "fastfall/render/DebugDraw.hpp"

#include "fmt/format.h"

#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

#if defined(__EMSCRIPTEN__)
#define FF_HAS_EMSCRIPTEN 1
#include <emscripten.h>
#include <emscripten/html5.h>

ff::EngineSettings engineDefaultSettings = {
    .allowMargins = true,
    .refreshRate = 0,
    .vsyncEnabled = false,
    .fullscreen = false,
    .showDebug = false,
};

#else
#define FF_HAS_EMSCRIPTEN 0

ff::EngineSettings engineDefaultSettings = {
    .allowMargins = true,
    .refreshRate = 0,
    .vsyncEnabled = true,
    .fullscreen = false,
    .showDebug = false,
    .runstyle = ff::EngineRunStyle::DoubleThread
};

#endif

#define IF_EMSCRIPTEN( expr ) if constexpr (FF_HAS_EMSCRIPTEN) { expr; }
#define IF_N_EMSCRIPTEN( expr ) if constexpr (!FF_HAS_EMSCRIPTEN) { expr; }


namespace ff {

Engine::Engine(Window* window)
    : Engine{ window, {GAME_W * 3, GAME_H * 3}, engineDefaultSettings }
{
}

Engine::Engine(Window* window, const Vec2u init_window_size, EngineSettings settings)
    : window{window}
    , initWinSize(init_window_size)
    , settings(settings)
    , ImGuiContent(ImGuiContentType::SIDEBAR_LEFT, "Engine", "System")
{
    assert(window);
    initRenderTarget(settings.fullscreen);

    initialized = true;
    if ((window && !window->valid())) {
        initialized = false;
    }

    debug::show = settings.showDebug;

    if (initialized) {
        LOG_INFO("Engine initialized");
    }
    else {
        LOG_INFO("Engine failed to initialize");
    }
}

Engine::~Engine() {
    LOG_INFO("Shutting down engine");
    debug::cleanup();
}

void Engine::push_runnable(EngineRunnable&& toRun) {
    runnables.push_back(std::forward<EngineRunnable>(toRun));
}

void Engine::drawRunnable(EngineRunnable& run) {
    ZoneScoped;
    TracyGpuZone("Draw Runnable");
    RenderTarget* target =
        run.getRTexture() ?
        static_cast<RenderTarget*>(run.getRTexture()) :
        static_cast<RenderTarget*>(window);

    auto activeState = run.getStateHandle().getActiveState();
    target->clear(activeState->getClearColor());

    target->draw(*activeState);

    // draw margin
    if (!run.getRTexture()) {
        View v = target->getView();
        target->setView(marginView);
        target->draw(*margins);
        target->setView(v);
    }
}

// -------------------------------------------

void Engine::prerun_init()
{
    ImGui_addContent();
    worldImgui.ImGui_addContent();
    input_cfg.ImGui_addContent();
    debugdrawImgui.ImGui_addContent();
    Resources::ImGui_init();

    if (window) {
        window->setActive();
    }

#ifdef DEBUG
    if (window != nullptr) {
        // ImGui::GetIO().IniFilename = NULL; // disable saving window positions


        ImGui::GetStyle().WindowRounding = 0.0f;
        ImGui::GetStyle().ChildRounding = 0.0f;
        ImGui::GetStyle().FrameRounding = 0.0f;
        ImGui::GetStyle().GrabRounding = 0.0f;
        ImGui::GetStyle().PopupRounding = 0.0f;
        ImGui::GetStyle().ScrollbarRounding = 0.0f;
        ImGui::GetStyle().TabRounding = 0.0f;
        ImGui::GetStyle().WindowBorderSize = 0.0f;

    }
#endif
}


bool Engine::run() {
    if (runnables.empty())
        return false;

#if FF_HAS_EMSCRIPTEN
    LOG_INFO("Engine loop config: emscripten");
    return run_emscripten();
#else
    switch (settings.runstyle) {
    case EngineRunStyle::SingleThread:
        LOG_INFO("Engine loop config: single thread");
        return run_singleThread();
    case EngineRunStyle::DoubleThread:
        LOG_INFO("Engine loop config: double thread");
        return run_doubleThread();
    default:
        return false;
    }
#endif
}

bool Engine::is_running() const noexcept { return running; };
bool Engine::is_init() const noexcept { return initialized; }

int Engine::get_window_scale() const noexcept { return windowZoom; }
const Window* Engine::get_window() const noexcept { return window; }
const FixedEngineClock& Engine::get_clock() const { return clock; }
secs Engine::get_uptime() const { return upTime; }

// -------------------------------------------

bool Engine::run_singleThread()
{
    prerun_init();
    SDL_SetThreadPriority(SDL_ThreadPriority::SDL_THREAD_PRIORITY_HIGH);

    running = true;
    bool first_frame = true;

    predrawRunnables();
    drawRunnables();

    while (is_running() && !runnables.empty())
    {
        profiler::curr_duration = {};
        profiler::frame_timer.reset();

        updateTimer();

        updateRunnables();
        profiler::curr_duration.update_time = profiler::frame_timer.elapsed();

        predrawRunnables();
        profiler::curr_duration.predraw_time = profiler::frame_timer.elapsed();

        updateView();
        drawRunnables();
        if (first_frame && window) {
            first_frame = false;
            window->showWindow();
        }
        profiler::curr_duration.draw_time = profiler::frame_timer.elapsed();

        updateImGui();
        profiler::curr_duration.imgui_time = profiler::frame_timer.elapsed();

        updateStateHandler();

        cleanRunnables();

        glDeleteStale();

        display();
        profiler::curr_duration.display_time = profiler::frame_timer.elapsed();

        sleep();
        profiler::curr_duration.sleep_time = profiler::frame_timer.elapsed();
        profiler::curr_duration.total_time = profiler::curr_duration.sleep_time;
        profiler::curr_duration.curr_uptime = upTime;
        profiler::curr_duration.curr_frame = clock.getTickCount();
        profiler::duration_buffer.add_time();
    }

    // clean up
    close();

    LOG_INFO("Run thread shutting down");

    running = false;

    glDeleteStale();

    return true;
}

// -------------------------------------------

bool Engine::run_doubleThread()
{

    prerun_init();
    SDL_SetThreadPriority(SDL_ThreadPriority::SDL_THREAD_PRIORITY_HIGH);

    std::barrier<> bar{ 2 };

    running = true;
    bool first_frame = true;

    predrawRunnables();
    drawRunnables();

    clock.reset();
    std::thread stateWorker(&Engine::runUpdate, this, &bar);


    while (is_running() && !runnables.empty()) {
        profiler::curr_duration = {};
        profiler::frame_timer.reset();

        updateTimer();


        bar.arrive_and_wait();

        // do update/draw

        updateView();
        drawRunnables();
        profiler::curr_duration.draw_time = profiler::frame_timer.elapsed();

        bar.arrive_and_wait();

		// predraw

        updateImGui();
        profiler::curr_duration.imgui_time = profiler::frame_timer.elapsed();

		// clean

        glDeleteStale();

        display();
        if (first_frame && window) {
            first_frame = false;
            window->showWindow();
        }
        FrameMark;
        profiler::curr_duration.display_time = profiler::frame_timer.elapsed();

        sleep();
        profiler::curr_duration.sleep_time = profiler::frame_timer.elapsed();
        profiler::curr_duration.total_time = profiler::curr_duration.sleep_time;
        profiler::curr_duration.curr_uptime = upTime;
        profiler::curr_duration.curr_frame = clock.getTickCount();
        profiler::duration_buffer.add_time();
    }

    // clean up
    close();

    LOG_INFO("Update thread shutting down");

    stateWorker.join();

    LOG_INFO("Display thread shutting down");

    running = false;

    glDeleteStale();

    return true;
}



void Engine::runUpdate(std::barrier<>* bar) {
    SDL_SetThreadPriority(SDL_ThreadPriority::SDL_THREAD_PRIORITY_HIGH);

    while (is_running() && !runnables.empty()) {

        bar->arrive_and_wait();


        // update/draw

        updateRunnables();
        profiler::curr_duration.update_time = profiler::frame_timer.elapsed();

        bar->arrive_and_wait();

		// predraw

        predrawRunnables();
        profiler::curr_duration.predraw_time = profiler::frame_timer.elapsed();

        updateStateHandler();

		// cleanup

        cleanRunnables();
    }
}

// -------------------------------------------

#if defined(__EMSCRIPTEN__)
EM_BOOL emscripten_resize(int event_type, const EmscriptenUiEvent* ui_event, void* user_data) {
    Engine* engine = (Engine*)user_data;
    double _width, _height;
    emscripten_get_element_css_size("#canvas", &_width, &_height);

    engine->resizeWindow(Vec2u{ (unsigned)_width, (unsigned)_height }, false);
    return true;
}
#endif

bool Engine::run_emscripten() {
#if defined(__EMSCRIPTEN__)

    prerun_init();

    running = true;

	clock.reset();

    // inform window of canvas size
    double _width, _height;
    emscripten_get_element_css_size("#canvas", &_width, &_height);
    resizeWindow(Vec2u{ (unsigned)_width, (unsigned)_height });
    window->setWindowSize( (unsigned)_width, (unsigned)_height );

    // canvas resize callback
    emscripten_set_resize_callback("#canvas", (void*)this, false, emscripten_resize);

	emscripten_set_main_loop_arg(Engine::emscripten_loop, (void*)this, 0, false);
#endif
    return true;

}

void Engine::emscripten_loop(void* engine_ptr) {
#if defined(__EMSCRIPTEN__)
	Engine* engine = (Engine*)engine_ptr;

	if(!engine->is_running() || engine->runnables.empty()) {
		return;
	}

    profiler::curr_duration = {};
    profiler::frame_timer.reset();

	engine->updateTimer();

    engine->updateRunnables();
    profiler::curr_duration.update_time = profiler::frame_timer.elapsed();

	engine->predrawRunnables();
    profiler::curr_duration.predraw_time = profiler::frame_timer.elapsed();

	engine->updateView();

	engine->drawRunnables();
    profiler::curr_duration.draw_time = profiler::frame_timer.elapsed();

	engine->updateImGui();
    profiler::curr_duration.imgui_time = profiler::frame_timer.elapsed();

	engine->updateStateHandler();

	engine->cleanRunnables();

	glDeleteStale();

    engine->display();
    profiler::curr_duration.display_time = profiler::frame_timer.elapsed();

	engine->sleep();
    profiler::curr_duration.sleep_time = profiler::frame_timer.elapsed();
    profiler::curr_duration.total_time = profiler::curr_duration.sleep_time;
    profiler::curr_duration.curr_uptime = engine->upTime;
    profiler::curr_duration.curr_frame = engine->clock.getTickCount();
    profiler::duration_buffer.add_time();
#endif
}

// -------------------------------------------

void Engine::close() {
    if (window) {
        window->showWindow(false);
    }
    InputConfig::closeJoystick();
    running = false;
}

// -------------------------------------------

void Engine::updateTimer() {
    ZoneScoped;

    tick = clock.tick();

    if (next_timescale) {
        clock.setTimescale(*next_timescale);
        next_timescale.reset();
    }

    if (window) {
        bool resetTimers = false;
        handleEvents(&resetTimers);
        if (resetTimers) {
			tick = clock.tick();
        }
    }

    if (ResourceWatcher::is_watch_running()) {
        if (Resources::reloadOutOfDateAssets()) {
           clock.tick();
        }
    }

    log::set_tick(clock.getTickCount());
    upTime += tick.elapsed;
    
}

void Engine::updateStateHandler() {
    ZoneScoped;
    for (auto& run : runnables) {
        run.getStateHandle().update();
    }
}

void Engine::updateView() {

    ZoneScoped;
    if (window) {
        if (!runnables.empty()) {
            View v = window->getView();
            EngineState *st = runnables.front().getStateHandle().getActiveState();
            Vec2f vPos = st->getViewPos();
            float vZoom = st->getViewZoom();

            v.setCenter(vPos);
            v.setSize(GAME_W_F * vZoom, GAME_H_F * vZoom);
            window->setView(v);
        }

        if (avgFPS != clock.getAvgFPS() || avgUPS != clock.getAvgUPS()) {
            avgFPS = clock.getAvgFPS();
            avgUPS = clock.getAvgUPS();
            std::string title = fmt::format("game v{}.{}.{} fps={} ups={}", VERSION[0], VERSION[1], VERSION[2], avgFPS, avgUPS);
            window->setWindowTitle(title);
        }
    }
}

void Engine::updateRunnables() 
{
    ZoneScoped;
    hasUpdated = tick.update_count > 0;
    while (tick.update_count > 0) {

        auto tickDuration = clock.upsDuration();

        if (pauseUpdate && !stepUpdate) {
            tickDuration = 0.0;
            tick.update_count = 1;
            hasUpdated = false;
        }
        stepUpdate = false;

        if (tickDuration > 0.0)
        {
            // TODO
            //Mouse::update(tickDuration);
            Mouse::update(tickDuration);
        }

        for (auto& run : runnables) {
            run.getStateHandle().getActiveState()->update(tickDuration);
        }
        tick.update_count--;
    }
}
void Engine::predrawRunnables() 
{
    ZoneScoped;
    float interp = interpolate ? tick.interp_value : 1.f;

    WindowState state {
        .scale = get_window_scale(),
        .window_size = Vec2u{ get_window()->getSize() }
    };

    predraw_state_t predraw_state{
        .interp    = interp,
        .updated   = hasUpdated,
        .update_dt = clock.upsDuration()
    };

    for (auto& run : runnables) {
        run.getStateHandle().getActiveState()->predraw(predraw_state, &state);
    }
    if (settings.showDebug) {
        debug::predraw(hasUpdated);
    }
    else {
        debug::reset();
    }
}

void Engine::drawRunnables() {
    // ZoneScoped;
    for (auto& run : runnables) {
        drawRunnable(run);
    }
    if (settings.showDebug) {
        debug::draw(*window);
    }
}

void Engine::updateImGui() {
#ifdef DEBUG
    ZoneScoped;
    if (window && settings.showDebug) {
        ImGuiNewFrame(*window);
        ImGuiFrame::getInstance().display(tick.elapsed);
        ImGuiRender();
    }
#endif
}

void Engine::cleanRunnables() {
    ZoneScoped;
    runnables.erase(
        std::remove_if(
            runnables.begin(), runnables.end(),
            [](EngineRunnable& r) {
                return r.isRunning();
            }
        ),
        runnables.end());

    if (!runnables.empty() && runnables.begin()->getRTexture() != nullptr) {
        runnables.clear();
    }
}


void Engine::display()
{
    // ZoneScoped;
    if (window) {
        window->display();
    }
}

void Engine::sleep() 
{
    ZoneScoped;
#if not defined(__EMSCRIPTEN__)
    clock.sleep();
#endif
}

// -------------------------------------------

void Engine::handleEvents(bool* timeWasted)
{
    ZoneScoped;
    // no window to handle inputs from
    if (window == nullptr)
        return;

    bool discardAllMousePresses = false;

    SDL_Event event;
    event_count = 0u;

    auto push_to_states = [this](SDL_Event event) {
        if (auto in = InputConfig::is_waiting_for_bind()) {
            switch (event.type) {
            case SDL_KEYDOWN:
                InputConfig::bindInput(*in, event.key.keysym.sym);
                return;
            case SDL_CONTROLLERBUTTONDOWN:
                InputConfig::bindInput(*in, InputConfig::GamepadInput::makeButton(event.cbutton.button));
                return;
            case SDL_CONTROLLERAXISMOTION:
                if (abs(event.caxis.value) > InputConfig::getAxisDeadzone()) {
                    bool which_side = event.caxis.value > 0;
                    InputConfig::bindInput(*in, InputConfig::GamepadInput::makeAxis(event.caxis.axis, which_side));
                    return;
                }
                break;
            }
        }

        for (auto &runnable: runnables) {
            if (auto state = runnable.getStateHandle().getActiveState();
                    state && state->pushEvent(event))
            {
                break;
            }
        }
    };

    while (SDL_PollEvent(&event)) 
    {
        event_count++;
        if (settings.showDebug && ImGui_ImplSDL2_ProcessEvent(&event)) {
            if (ImGui::GetIO().WantCaptureMouse && (event.type & SDL_MOUSEMOTION) > 0) {
                continue;
            }
        }

        if (discardAllMousePresses && (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP))
        {
            continue;
        }

        switch (event.type) {
        case SDL_WINDOWEVENT:
            switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                close();
                break;
            case SDL_WINDOWEVENT_RESIZED:
                if (!settings.fullscreen) {
                    //LOG_INFO("resize: {}", Vec2u(window->getSize()));
                    resizeWindow(Vec2u(window->getSize()));
                    *timeWasted = true;
                }
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                hasFocus = true;
                *timeWasted = true;
                discardAllMousePresses = true;
                Mouse::reset();
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                hasFocus = false;
                Mouse::reset();
                break;
            }
            break;
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: 
                if (!settings.fullscreen) {
                    IF_N_EMSCRIPTEN(close());
                }
                else {
                    setFullscreen(!settings.fullscreen);
                }
                break;
            case SDLK_F10: 
                setFullscreen(!settings.fullscreen);
                break;
            case SDLK_KP_1:
                if (isFrozen()) {
                    unfreeze();
                }
                else {
                    freeze();
                }
                break;
            case SDLK_KP_2:
                freezeStepOnce();
                break;
            case SDLK_KP_3: {
                settings.showDebug = !settings.showDebug;
                unsigned scale = settings.showDebug ? 2 : 1;
                IF_N_EMSCRIPTEN(SDL_SetWindowMinimumSize(window->getSDL_Window(), GAME_W * scale, GAME_H * scale));
                resizeWindow(Vec2u(window->getSize()));
                debug::show = settings.showDebug;
                LOG_INFO("Toggling debug mode");
                break;
            }
            default:
                push_to_states(event);
            }
            break;
        case SDL_KEYDOWN:
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
        case SDL_CONTROLLERAXISMOTION:
            push_to_states(event);
            break;
        case SDL_JOYDEVICEADDED:
        case SDL_JOYDEVICEREMOVED:
        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
            InputConfig::updateJoystick();
            break;
        case SDL_MOUSEMOTION:
            Mouse::set_window_pos({event.motion.x, event.motion.y});
            break;
        case SDL_MOUSEBUTTONDOWN: {
            Vec2i pos{event.button.x, event.button.y};
            if (event.button.button == SDL_BUTTON_LEFT) {
                Mouse::notify_down(Mouse::MButton::Left, pos);
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                Mouse::notify_down(Mouse::MButton::Right, pos);
            }
            break;
        }
        case SDL_MOUSEBUTTONUP: {
            Vec2i pos{event.button.x, event.button.y};
            if (event.button.button == SDL_BUTTON_LEFT) {
                Mouse::notify_up(Mouse::MButton::Left, pos);
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                Mouse::notify_up(Mouse::MButton::Right, pos);
            }
            break;
        }

        }
    }

    if (window) {
        // check if mouse is outside the window
        auto window_pos = Mouse::window_pos();
        if (window_pos.x == 0 || window_pos.x == window->getSize().x - 1
            || window_pos.y == 0 || window_pos.y == window->getSize().y - 1)
        {
            Vec2i g_mpos;
            SDL_GetGlobalMouseState(&g_mpos.x, &g_mpos.y);
#if !defined(__EMSCRIPTEN__)
            Mouse::set_window_pos(g_mpos - Vec2i{ window->getPosition() });
#else
            Mouse::set_window_pos(g_mpos);
#endif
        }

        Mouse::update_view(window->getView());
    }
}

void Engine::initRenderTarget(bool fullscreen)
{
    settings.fullscreen = fullscreen;

    SDL_DisplayMode mode;
    int r = SDL_GetCurrentDisplayMode(0, &mode);
    assert(r == 0);

    Vec2u displaySize(mode.w, mode.h);

    window->setWindowTitle("");

    if (settings.fullscreen) {
        // go borderless fullscreen
        window->setWindowSize(displaySize);
        window->setWindowFullscreen(Window::FullscreenType::FULLSCREEN_DESKTOP);
    }
    else {
        window->setWindowSize(initWinSize);
        window->setWindowCentered();
        window->setWindowResizable();
    }
    window->setActive();

    // browser handles vsync if emscripten
    IF_N_EMSCRIPTEN(window->setVsyncEnabled(settings.vsyncEnabled));

    //window->showWindow();

    margins = std::make_unique<VertexArray>(Primitive::TRIANGLE_STRIP, 10);
    for (int i = 0; i < 10; i++) {
        (*margins.get())[i].color = Color::Black;
    }

    unsigned scale = settings.showDebug ? 2 : 1;
    IF_N_EMSCRIPTEN(SDL_SetWindowMinimumSize(window->getSDL_Window(), GAME_W * scale, GAME_H * scale));

#if !defined(__EMSCRIPTEN__)
    resizeWindow(Vec2u(window->getSize()));
#endif
}

bool Engine::setFullscreen(bool fullscreen) {
    if (!window || settings.fullscreen == fullscreen)
        return false;

    settings.fullscreen = fullscreen;

    LOG_INFO(settings.fullscreen ? "Entering fullscreen" : "Exiting fullscreen");

    SDL_DisplayMode mode;
    int r = SDL_GetCurrentDisplayMode(0, &mode);
    assert(r == 0);

    if (settings.fullscreen) {
        lastWindowedPos = Vec2i(window->getPosition().x, window->getPosition().y);
        lastWindowedSize = window->getSize();

        Vec2i displaySize{mode.w, mode.h};

        window->setWindowFullscreen(Window::FullscreenType::FULLSCREEN_DESKTOP);
    }
    else {
        // go windowed

        window->setWindowFullscreen(Window::FullscreenType::WINDOWED);
        window->setWindowPosition(lastWindowedPos);
        window->setWindowResizable();
    }
    window->setActive();
    window->setVsyncEnabled(settings.vsyncEnabled);

    resizeWindow(Vec2u(window->getSize()));
    return true;
}

void Engine::resizeWindow(Vec2u size, bool force_size)
{
    if (!window)
        return;

    bool expandMargins = settings.showDebug && settings.allowMargins;
    ImGuiFrame::getInstance().setDisplay(expandMargins);

    Vec2u minSize{ expandMargins ? Vec2u{ GAME_W * 2, GAME_H * 2 } : Vec2u{ GAME_W, GAME_H } };


    Vec2u finalSize{ size };
    if (!force_size) {
        finalSize.x = (std::max)(size.x, minSize.x);
        finalSize.y = (std::max)(size.y, minSize.y);
    }

    int scale = static_cast<int>((std::min)(floor((float)finalSize.x / GAME_W), floor((float)finalSize.y / GAME_H)));

    if (expandMargins)
        scale = (std::max)(1, scale - 1);

    if (window) {
        if (finalSize.x != size.x || finalSize.y != size.y) {
            window->setWindowSize(finalSize);
        }
        if (!settings.allowMargins && !settings.fullscreen) {
            finalSize.x = GAME_W * scale;
            finalSize.y = GAME_H * scale;
            window->setWindowSize(finalSize);
        }
    }

    //reposition content sprite
    unsigned int trueX = GAME_W * scale;
    unsigned int trueY = GAME_H * scale;

    int offX = ((finalSize.x - trueX) / 2);
    int offY = (finalSize.y - trueY) / 2;

    View v = window->getView();

    v.setViewport({
        (float)offX,
        (float)offY,
        (float)trueX,
        (float)trueY
        });

    v.setSize(GAME_W_F, GAME_H_F);

    window->setView(v);

    windowZoom = scale;

    //update margins

    glm::fvec4 vp = window->getView().getViewport();
    Recti viewport{ 
        (int)(vp[0]), 
        (int)(vp[1]),
        (int)(vp[2]),
        (int)(vp[3])
    };
    Recti windowSize(0, 0, finalSize.x, finalSize.y);

    Rectf vpf {viewport};
    Rectf wsf {windowSize};

    auto& margin = *margins;
    margin[0] = { math::rect_topleft(wsf)  };
    margin[1] = { math::rect_topleft(vpf)  };
    margin[2] = { math::rect_botleft(wsf)  };
    margin[3] = { math::rect_botleft(vpf)  };
    margin[4] = { math::rect_botright(wsf) };
    margin[5] = { math::rect_botright(vpf) };
    margin[6] = { math::rect_topright(wsf) };
    margin[7] = { math::rect_topright(vpf) };
    margin[8] = { math::rect_topleft(wsf)  };
    margin[9] = { math::rect_topleft(vpf)  };

    marginView = View{ { 0.f, 0.f }, Vec2f{ finalSize } };
    marginView.setCenter(Vec2f{ finalSize } / 2.f);
    
    if (ImGuiFrame::getInstance().isDisplay()) {
        ImGuiFrame::getInstance().resize(windowSize, Vec2u(viewport.getSize().x, viewport.getSize().y));
    }
}

bool showImGuiDemo = false;
bool showImPlotDemo = false;

void Engine::ImGui_getContent(secs deltaTime) {

    if (ImGui::Button("Show ImGui Demo")) {
        showImGuiDemo = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Show ImPlot Demo")) {
        showImPlotDemo = true;
    }

    IF_N_EMSCRIPTEN(
        static bool wireframe_enabled = false;
        if(ImGui::Checkbox("Wireframe", &wireframe_enabled)) {
            glCheck(glPolygonMode(GL_FRONT_AND_BACK, wireframe_enabled ? GL_LINE : GL_FILL));
        }
    );

    ImGui::BulletText("Window");
    ImGui::Text("Window Vertex Counter = %4zu", window->getVertexCounter());
    ImGui::Text("Window Draw Counter = %4zu", window->getDrawCallCounter());
    ImGui::Text("Window Pos   = (%4d, %4d)", window->getPosition().x, window->getPosition().y);
    ImGui::Text("Window Size  = (%4d, %4d)", window->getSize().x, window->getSize().y);
    bool goFullscreen = settings.fullscreen;
    if (ImGui::Checkbox("Fullscreen", &goFullscreen)) {
        setFullscreen(goFullscreen);
    }
    ImGui::Separator();
    glm::fvec4 vp = window->getView().getViewport();
    ImGui::Text("Viewport      = (%6.2f, %6.2f, %6.2f, %6.2f)", vp[0], vp[1], vp[2], vp[3]);
    ImGui::Text("Zoom          =  %2dx", windowZoom);
    ImGui::Text("Cursor (game) = (%6.2f, %6.2f)", Mouse::world_pos().x, Mouse::world_pos().y);
    ImGui::Text("Cursor inside = %s", Mouse::in_view() ? "true" : "false");
    ImGui::Separator();
    ImGui::Text("Events  = %4d", event_count);

    profiler::enable = false;
    if (ImGui::CollapsingHeader("Framerate Graph", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const auto& buff = profiler::duration_buffer;
        if (ImPlot::BeginPlot("##FPS", ImVec2(-1, 150),
            ImPlotFlags_NoTitle | ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMouseText))
        {
            profiler::enable = true;

            static double xmin = -1.0;
            static double xmax = 0.0;

            if (!buff.empty() && profiler::enable) {
                xmin = buff.back().curr_uptime - profiler::DurationBuffer::CYCLE_DURATION;
                xmax = buff.back().curr_uptime;
            }

            static double ymin = 0.0;
            static double ymax = 60.0;

            if (!buff.empty() && profiler::enable) {
                ymin = 0.0;
                ymax = (clock.getTargetFPS() == FixedEngineClock::FPS_UNLIMITED ? clock.getAvgFPS() : clock.getTargetFPS());
            }

            ImPlot::SetupAxesLimits(xmin, xmax, ymin, 1.5 / ymax, ImPlotCond_Always);

            static ImPlotFormatter format = [](double value, char* buff, int size, void* userdata)
            {
                return snprintf(buff, size, "%.2fms", value * 1000.0);
            };

            ImPlot::SetupAxisFormat(ImAxis_Y1, format);
            ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 1.f);

            const auto plot = [&](std::string_view name, const secs* start) {
                ImPlot::PlotShaded(name.data(), &buff[0].curr_uptime, start, buff.size(), 0.0, 0, 0, sizeof(profiler::Duration));
            };

            if (!buff.empty()) {
                plot("sleep",   &buff[0].sleep_time);
                plot("display", &buff[0].display_time);
                plot("imgui",   &buff[0].imgui_time);
                plot("draw",    &buff[0].draw_time);
                plot("predraw", &buff[0].predraw_time);
                ImPlot::SetNextFillStyle(ImVec4(1.f, 1.f, 1.f, 1.f));
                plot("update",  &buff[0].update_time);
            }

            ImPlot::PopStyleVar();
            ImPlot::EndPlot();
        }
    }

    ImGui::Text("| FPS:%4d |", clock.getAvgFPS());
    ImGui::SameLine();
    ImGui::Text("Tick#:%6zu | ", clock.getTickCount());

    ImGui::Separator();

    ImGui::Checkbox("Interpolate", &interpolate);
    ImGui::SameLine();
    ImGui::Text("%1.3f", tick.interp_value);

    if (ImGui::Checkbox("Freeze", &pauseUpdate)) {
        if (!pauseUpdate)
            unfreeze();
        else
            freeze();
    }
    ImGui::SameLine();
    ImGui::SameLine();
    if (ImGui::Button("Step")) {
        freezeStepOnce();
    }

    IF_N_EMSCRIPTEN(
        if (ImGui::Checkbox("Vsync Enabled", &settings.vsyncEnabled)) {
            window->setVsyncEnabled(settings.vsyncEnabled);
        }
    );

    int fps = clock.getTargetFPS();
    constexpr std::string_view show_inf = "UNLIMITED";
    constexpr std::string_view show_fps = "%d";

    IF_N_EMSCRIPTEN(
        if (ImGui::DragInt("FPS", &fps, 5, 0, 500, (fps == 0 ? show_inf : show_fps).data())) {
            settings.refreshRate = fps;
            clock.setTargetFPS(fps);
        }
    );

    int ups = clock.getTargetUPS();
    if (ImGui::DragInt("UPS", &ups, 5, 10, 500, "%d")) {
        clock.setTargetUPS(ups);
    }

    static float speed = 1.f;
    if (ImGui::DragFloat("Timescale", &speed, 0.0005f, 0.f, 5.0)) {
        next_timescale = speed;
    }

    if (ImGui::SmallButton("0.01x")) { speed = 0.01f; next_timescale = speed; } ImGui::SameLine();
    if (ImGui::SmallButton("0.10x")) { speed = 0.10f; next_timescale = speed; } ImGui::SameLine();
    if (ImGui::SmallButton("0.50x")) { speed = 0.50f; next_timescale = speed; } ImGui::SameLine();
    if (ImGui::SmallButton("1.00x")) { speed = 1.00f; next_timescale = speed; } ImGui::SameLine();
    if (ImGui::SmallButton("2.00x")) { speed = 2.00f; next_timescale = speed; } ImGui::SameLine();
    if (ImGui::SmallButton("5.00x")) { speed = 5.00f; next_timescale = speed; }
}

void Engine::ImGui_getExtraContent() {
    if (showImGuiDemo) ImGui::ShowDemoWindow(&showImGuiDemo);
    if (showImPlotDemo) ImPlot::ShowDemoWindow(&showImPlotDemo);
}

void Engine::freeze() {
    log::set_verbosity(log::level::STEP);  pauseUpdate = true; stepUpdate = false;
};
void Engine::freezeStepOnce() {
    log::set_verbosity(log::level::STEP); pauseUpdate = true; stepUpdate = true; LOG_STEP("STEP");
};
void Engine::unfreeze() {
    log::set_verbosity(log::level::VERB); pauseUpdate = false; stepUpdate = false;
};
bool Engine::isFrozen() const noexcept {
    return pauseUpdate;
};

DebugDrawImgui::DebugDrawImgui() :
    ImGuiContent(ImGuiContentType::SIDEBAR_LEFT, "Debug", "System")
{

}
void DebugDrawImgui::ImGui_getContent(secs deltaTime) {
    ImGui::Checkbox("Show Debug", &debug::show);
    ImGui::Checkbox("Darken Screen", &debug::darken);

    ImGui::Separator();
    if (ImGui::Button("Enable All")) {
        debug::set_all(true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Disable All")) {
        debug::set_all(false);
    }

    for (auto t : debug::types) {
        bool on = debug::type_enabled(t);
        if (ImGui::Checkbox(debug::to_str(t).data(), &on)) {
            debug::set(t, on);
        }
    }

    ImGui::Separator();
    ImGui::Text("Draw calls: %zu", debug::stats.draw_calls);
    ImGui::Text("Vertices:   %zu", debug::stats.vertices);
    ImGui::Text("Repeats:    %zu", debug::stats.repeat_calls);

}


}
