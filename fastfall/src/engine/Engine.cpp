
#include <queue>
#include <memory>
#include <thread>
#include <algorithm>
#include <atomic>

#include "fastfall/engine/imgui/ImGuiFrame.hpp"

#include "fastfall/util/log.hpp"

#include "fastfall/render.hpp"
#include "../render/detail/error.hpp"

#include "fastfall/engine/Engine.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/engine/input.hpp"

#include "fastfall/resource/ResourceWatcher.hpp"

#include <iostream>

#include "fastfall/resource/Resources.hpp"
#include "fastfall/render/AnimatedSprite.hpp"
#include "fastfall/render/DebugDraw.hpp"

#include "fmt/format.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

namespace profiler {

    struct Duration
    {
        size_t  curr_frame;
        secs    curr_uptime;

        secs    update_time;
        secs    predraw_time;
        secs    draw_time;
        secs    imgui_time;
        secs    display_time;
        secs    sleep_time;
        secs    total_time;
    };

    bool enable = false;

    struct DurationBuffer
    {
    public:
        void add_time(Duration d)
        {
#if DEBUG
            if (enable) {
                buffer.push_back(d);
                update_past();
                cycle_durations();
            }
#endif
        }

        auto begin() const { return onesec_past; };
        auto end() const { return buffer.cend(); }
        auto& back() const { return buffer.back(); };
        auto size() const { return buffer.empty() ? 0 : static_cast<size_t>(end() - begin()); }
        bool empty() const { return size() == 0; }

        const Duration& operator[] (size_t ndx) const { return begin()[ndx]; }

        constexpr static secs CYCLE_DURATION = 3.0;

    private:

        std::vector<Duration> buffer;
        std::vector<Duration>::iterator onesec_past;

        void cycle_durations()
        {
            if (!buffer.empty()
                && buffer.front().curr_uptime + CYCLE_DURATION * 2.0 <= buffer.back().curr_uptime)
            {
                // rotate [onesec_past, back] to the front, erase the rest
                // this way no reallocation occurs
                buffer.erase(
                    std::rotate(buffer.begin(), onesec_past, buffer.end()), 
                    buffer.end()
                );

                // reset onesec_past
                onesec_past = buffer.begin();
            }
        }

        void update_past()
        {
            onesec_past = std::lower_bound(buffer.begin(), buffer.end(), buffer.back().curr_uptime - CYCLE_DURATION,
                [](const Duration& b, secs a) {
                    return b.curr_uptime < a;
                });
        }
    };


    struct Timer
    {
        std::chrono::steady_clock clock;
        std::chrono::time_point<std::chrono::steady_clock> start_time;

        Timer()
        {
            start_time = clock.now();
        }

        secs elapsed() const {
            return std::chrono::duration<secs>{ clock.now() - start_time }.count();
        }

        secs reset() {
            secs time = elapsed();
            start_time = clock.now();
            return time;
        }
    };

    DurationBuffer duration_buffer;
    Duration curr_duration;
    Timer frame_timer;
}


namespace ff {

std::unique_ptr<Engine> Engine::engineInstance;

void Engine::init(std::unique_ptr<Window>&& window, EngineRunnable&& toRun, const Vec2u& initWindowSize, EngineSettings engineSettings) {
    LOG_INFO("Initializing engine instance");
    engineInstance = std::unique_ptr<Engine>(new Engine{ std::move(window), std::move(toRun), initWindowSize, engineSettings });
    LOG_INFO("Initialization complete");
}

void Engine::shutdown() {
    LOG_INFO("Shutting down engine instance");
    engineInstance.reset();
    LOG_INFO("Shutdown complete, have a nice day");
}

Engine::Engine(
    std::unique_ptr<Window>&& initWindow,
    EngineRunnable&& toRun,
    const Vec2u& initWindowSize,
    EngineSettings engineSettings
) 
    : window{std::move(initWindow)}
    , initWinSize(initWindowSize)
    , settings(engineSettings)
    , ImGuiContent(ImGuiContentType::SIDEBAR_LEFT, "Engine", "System")
{
    // use first runnable to determine if we need a window
    addRunnable(std::move(toRun));
    initRenderTarget(settings.fullscreen);

    initialized = true;
    if (runnables.empty() || (window && !window->valid())) {
        initialized = false;
    }

    debug_draw::enable(settings.showDebug);
}

void Engine::addRunnable(EngineRunnable&& toRun) {
    runnables.push_back(std::move(toRun));
}

void Engine::drawRunnable(EngineRunnable& run) {
    RenderTarget* target =
        run.getRTexture() ?
        static_cast<RenderTarget*>(run.getRTexture()) :
        static_cast<RenderTarget*>(window.get());

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
    instanceObs.ImGui_addContent();
    input.ImGui_addContent();
    debugdrawImgui.ImGui_addContent();

    if (window) {
        window->setActive();
    }

#ifdef DEBUG
    if (window != nullptr) {
        ImGui::GetIO().IniFilename = NULL; // disable saving window positions


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
    switch (settings.runstyle) {
    case EngineRunStyle::SingleThread:  return run_singleThread();
    case EngineRunStyle::DoubleThread:  return run_doubleThread();
    case EngineRunStyle::Emscripten:    return run_emscripten();
    default: return false;
    }
}

// -------------------------------------------

bool Engine::run_singleThread()
{
    prerun_init();
    SDL_SetThreadPriority(SDL_ThreadPriority::SDL_THREAD_PRIORITY_HIGH);

    running = true;

    while (isRunning() && !runnables.empty()) 
    {
        profiler::curr_duration = {};
        profiler::frame_timer.reset();

        updateTimer();

        Input::update(tick.elapsed);

        updateRunnables();
        profiler::curr_duration.update_time = profiler::frame_timer.elapsed();

        predrawRunnables();
        profiler::curr_duration.predraw_time = profiler::frame_timer.elapsed();

        updateView();

        drawRunnables();
        profiler::curr_duration.draw_time = profiler::frame_timer.elapsed();

        updateImGui();
        profiler::curr_duration.imgui_time = profiler::frame_timer.elapsed();

        updateStateHandler();

        cleanRunnables();

        ff::glDeleteStale();

        display();
        profiler::curr_duration.display_time = profiler::frame_timer.elapsed();

        sleep();
        profiler::curr_duration.sleep_time = profiler::frame_timer.elapsed();
        profiler::curr_duration.total_time = profiler::curr_duration.sleep_time;
        profiler::curr_duration.curr_uptime = upTime;
        profiler::curr_duration.curr_frame = clock.getTickCount();
        profiler::duration_buffer.add_time(profiler::curr_duration);
    }

    // clean up
    close();

    LOG_INFO("Run thread shutting down");

    running = false;

    ff::glDeleteStale();

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
    clock.reset();
    std::thread stateWorker(&Engine::runUpdate, this, &bar);

    while (isRunning() && !runnables.empty()) {
        profiler::curr_duration = {};
        profiler::frame_timer.reset();

        updateTimer();


        bar.arrive_and_wait();

        // do update/draw

        if (!first_frame) {
            updateView();
            drawRunnables();
            profiler::curr_duration.draw_time = profiler::frame_timer.elapsed();
        }
        else {
            first_frame = false;
        }

        bar.arrive_and_wait();

		// predraw

        updateImGui();
        profiler::curr_duration.imgui_time = profiler::frame_timer.elapsed();

		// clean

        ff::glDeleteStale();

        display();
        profiler::curr_duration.display_time = profiler::frame_timer.elapsed();

        sleep();
        profiler::curr_duration.sleep_time = profiler::frame_timer.elapsed();
        profiler::curr_duration.total_time = profiler::curr_duration.sleep_time;
        profiler::curr_duration.curr_uptime = upTime;
        profiler::curr_duration.curr_frame = clock.getTickCount();
        profiler::duration_buffer.add_time(profiler::curr_duration);
    }

    // clean up
    close();

    LOG_INFO("Update thread shutting down");

    stateWorker.join();

    LOG_INFO("Display thread shutting down");

    running = false;

    ff::glDeleteStale();

    return true;
}



void Engine::runUpdate(std::barrier<>* bar) {
    SDL_SetThreadPriority(SDL_ThreadPriority::SDL_THREAD_PRIORITY_HIGH);

    while (isRunning() && !runnables.empty()) {

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

bool Engine::run_emscripten() {
#if defined(__EMSCRIPTEN__)

    prerun_init();

    running = true;

	clock.reset();

	emscripten_set_main_loop_arg(Engine::emscripten_loop, (void*)this, 0, false);
#endif
    return true;

}

void Engine::emscripten_loop(void* engine_ptr) {
	Engine* engine = (Engine*)engine_ptr;

	if(!engine->isRunning() || engine->runnables.empty())
		return;

    profiler::curr_duration = {};
    profiler::frame_timer.reset();

	engine->updateTimer();

	Input::update(engine->tick.elapsed);

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

	ff::glDeleteStale();

    engine->display();
    profiler::curr_duration.display_time = profiler::frame_timer.elapsed();

	engine->sleep();
    profiler::curr_duration.sleep_time = profiler::frame_timer.elapsed();
    profiler::curr_duration.total_time = profiler::curr_duration.sleep_time;
    profiler::curr_duration.curr_uptime = engine->upTime;
    profiler::curr_duration.curr_frame = engine->clock.getTickCount();
    profiler::duration_buffer.add_time(profiler::curr_duration);
}

// -------------------------------------------

void Engine::close() {
    if (window) {
        window->showWindow(false);
    }
    Input::closeJoystick();
    running = false;
}

// -------------------------------------------

void Engine::updateTimer() {

    tick = clock.tick();

    if (window) {
        bool resetTimers = false;
        handleEvents(&resetTimers);
        if (resetTimers) {
			tick = clock.tick();
        }
    }

    if (ResourceWatcher::is_watch_running()) {
        if (Resources::reloadOutOfDateAssets()) {
           // clock.tick();
        }
    }

    log::set_tick(clock.getTickCount());
    upTime += tick.elapsed;
    
}

void Engine::updateStateHandler() {
    for (auto& run : runnables) {
        run.getStateHandle().update();
    }
}

void Engine::updateView() {

    if (window) {
        View v = window->getView();
        EngineState* st = runnables.front().getStateHandle().getActiveState();
        Vec2f vPos = st->getViewPos();
        float vZoom = st->getViewZoom();

        v.setCenter(vPos);
        v.setSize(GAME_W_F * vZoom, GAME_H_F * vZoom);
        window->setView(v);


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
    hasUpdated = tick.update_count > 0;
    while (tick.update_count > 0) {

        auto tickDuration = clock.upsDuration();

        if (pauseUpdate && !stepUpdate) {
            tickDuration = 0.0;
            tick.update_count = 1;
        }
        stepUpdate = false;

        if (tickDuration > 0.0)
        {
            Input::update(tickDuration);
        }

        for (auto& run : runnables) {
            run.getStateHandle().getActiveState()->update(tickDuration);
        }
        tick.update_count--;
    }
}
void Engine::predrawRunnables() 
{
    static bool latch_interp = false;
    float interp = tick.interp_value;

    if (pauseUpdate && hasUpdated) {
        pauseInterpolation = true;
    }
    else if (!pauseUpdate) {
        pauseInterpolation = false;
    }

    if (pauseInterpolation) {
        interp = 1.f;
    }

    for (auto& run : runnables) {
        run.getStateHandle().getActiveState()->predraw(interp, hasUpdated);
    }
    if (settings.showDebug) {
        if (hasUpdated) {
            debug_draw::swapDrawLists();
        }
    }
    else {
        debug_draw::clear();
    }
}

void Engine::drawRunnables() {
    for (auto& run : runnables) {
        drawRunnable(run);
    }
    if (settings.showDebug)
        debug_draw::draw(*window);
}

void Engine::updateImGui() {
#ifdef DEBUG
    if (window && settings.showDebug) {
        ff::ImGuiNewFrame(*window);
        ImGuiFrame::getInstance().display();
        ff::ImGuiRender();
    }
#endif
}

void Engine::cleanRunnables() {
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
    if (window) {
        window->display();
    }
}

void Engine::sleep() 
{


#if not defined(__EMSCRIPTEN__)
    clock.sleep();
#endif
}

// -------------------------------------------

void Engine::handleEvents(bool* timeWasted)
{

    // no window to handle inputs from
    if (window == nullptr)
        return;

    bool discardMousePress = false;

    SDL_Event event;
    event_count = 0u;

    while (SDL_PollEvent(&event)) 
    {

        event_count++;

        if (ImGui_ImplSDL2_ProcessEvent(&event)) {
            if (ImGui::GetIO().WantCaptureMouse && (event.type & SDL_MOUSEMOTION) > 0) {
                continue;
            }
        }

        switch (event.type) {
        case SDL_WINDOWEVENT:
            switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                close();
                break;
            case SDL_WINDOWEVENT_RESIZED:
                if (!settings.fullscreen) {
                    resizeWindow(Vec2u(window->getSize()));
                    *timeWasted = true;
                }
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                hasFocus = true;
                *timeWasted = true;
                discardMousePress = true;
                Input::resetState();
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                hasFocus = false;
                Input::resetState();
                break;
            }
            break;
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: 
                if (!settings.fullscreen) {
#if not defined(__EMSCRIPTEN__)
                    close();
#endif
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
            case SDLK_KP_3:
                settings.showDebug = !settings.showDebug;
                unsigned scale = settings.showDebug ? 2 : 1;
#if not defined(__EMSCRIPTEN__)
                SDL_SetWindowMinimumSize(window->getSDL_Window(), GAME_W * scale, GAME_H * scale);
#endif
                resizeWindow(Vec2u(window->getSize()));
                debug_draw::enable(settings.showDebug);
                LOG_INFO("Toggling debug mode");
                break;
            }
            [[fallthrough]];
        default:
            if (!discardMousePress || (event.type != SDL_MOUSEBUTTONDOWN && event.type != SDL_MOUSEBUTTONUP))
                Input::pushEvent(event);
            break;
        }
    }

    if (window) {
        Vec2i pixel_pos = Input::getMouseWindowPosition();
        Vec2f world_pos = Vec2f{ window->windowCoordToWorld(pixel_pos) };

        glm::vec4 viewport = window->getView().getViewport();
        bool inside = pixel_pos.x >= viewport[0] 
            && pixel_pos.x <= (viewport[0] + viewport[2])
            && pixel_pos.y >= viewport[1]
            && pixel_pos.y <= (viewport[1] + viewport[3]);


        Input::setMouseWorldPosition(world_pos);
        Input::setMouseInView(inside);

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
#if not defined(__EMSCRIPTEN__)
	// browser handles vsync if emscripten
    window->setVsyncEnabled(settings.vsyncEnabled);
#endif
    window->showWindow();

    margins = std::make_unique<VertexArray>(Primitive::TRIANGLE_STRIP, 10);
    for (int i = 0; i < 10; i++) {
        (*margins.get())[i].color = Color::Black;
    }

    unsigned scale = settings.showDebug ? 2 : 1;
#if not defined(__EMSCRIPTEN__)
    SDL_SetWindowMinimumSize(window->getSDL_Window(), GAME_W * scale, GAME_H * scale);
#endif

    resizeWindow(Vec2u(window->getSize()));
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

void Engine::resizeWindow(
    Vec2u size)
{
    if (!window)
        return;


    bool expandMargins = settings.showDebug && settings.allowMargins;
    ImGuiFrame::getInstance().setDisplay(expandMargins);

    Vec2u minSize{ expandMargins ? Vec2u{ GAME_W * 2, GAME_H * 2 } : Vec2u{ GAME_W, GAME_H } };


    Vec2u finalSize;
    finalSize.x = std::max(size.x, minSize.x);
    finalSize.y = std::max(size.y, minSize.y);


    int scale = static_cast<int>(std::min(floor((float)finalSize.x / GAME_W), floor((float)finalSize.y / GAME_H)));

    if (expandMargins)
        scale = std::max(1, scale - 1);

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
    Recti windowSize(0, 0, window->getSize().x, window->getSize().y);

    (*margins.get())[0].pos = glm::fvec2((float)windowSize.left, (float)windowSize.top);
    (*margins.get())[1].pos = glm::fvec2((float)viewport.left,   (float)viewport.top);
    (*margins.get())[2].pos = glm::fvec2((float)windowSize.left, (float)windowSize.top + windowSize.height);
    (*margins.get())[3].pos = glm::fvec2((float)viewport.left,   (float)viewport.top + viewport.height);
    (*margins.get())[4].pos = glm::fvec2((float)windowSize.left + windowSize.width, (float)windowSize.top + windowSize.height);
    (*margins.get())[5].pos = glm::fvec2((float)viewport.left + viewport.width, (float)viewport.top + viewport.height);
    (*margins.get())[6].pos = glm::fvec2((float)windowSize.left + windowSize.width, (float)windowSize.top);
    (*margins.get())[7].pos = glm::fvec2((float)viewport.left + viewport.width, (float)viewport.top);
    (*margins.get())[8].pos = glm::fvec2((float)windowSize.left, (float)windowSize.top);
    (*margins.get())[9].pos = glm::fvec2((float)viewport.left, (float)viewport.top);

    marginView = View{ { 0.f, 0.f }, window->getSize() };
    marginView.setCenter(glm::fvec2{ window->getSize() } / 2.f);
    
    if (ImGuiFrame::getInstance().isDisplay()) {
        ImGuiFrame::getInstance().resize(windowSize, Vec2u(viewport.getSize().x, viewport.getSize().y));
    }
    
}

bool showImGuiDemo = false;
bool showImPlotDemo = false;

void Engine::ImGui_getContent() {

    if (ImGui::Button("Show ImGui Demo")) {
        showImGuiDemo = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Show ImPlot Demo")) {
        showImPlotDemo = true;
    }


#if not defined(__EMSCRIPTEN__)
	static bool wireframe_enabled = false;
	if(ImGui::Checkbox("Wireframe", &wireframe_enabled)) {
		glCheck(glPolygonMode(GL_FRONT_AND_BACK, wireframe_enabled ? GL_LINE : GL_FILL));
	}
#endif

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
    ImGui::Text("Cursor (game) = (%6.2f, %6.2f)", Input::getMouseWorldPosition().x, Input::getMouseWorldPosition().y);
    ImGui::Text("Cursor inside = %s", Input::getMouseInView() ? "true" : "false");
    ImGui::Separator();
    ImGui::Text("Events  = %4d", event_count);

    profiler::enable = false;
    if (ImGui::CollapsingHeader("Framerate Graph", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const auto& buff = profiler::duration_buffer;
        if (ImPlot::BeginPlot("##FPS", NULL, NULL, ImVec2(-1, 150), 
            ImPlotFlags_NoTitle | ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMouseText,
            ImPlotAxisFlags_None, 0))
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
                snprintf(buff, size, "%.2fms", value * 1000.0);
            };

            ImPlot::SetupAxisFormat(ImAxis_Y1, format);
            ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 1.f);

            const auto plot = [&](std::string_view name, const secs* start) {
                ImPlot::PlotShaded(name.data(), &buff[0].curr_uptime, start, buff.size(), 0.0, 0, sizeof(profiler::Duration));
            };

            if (!buff.empty()) {
                plot("sleep",   &buff[0].sleep_time);
                plot("display", &buff[0].display_time);
                plot("imgui",   &buff[0].imgui_time);
                plot("draw",    &buff[0].draw_time);
                plot("predraw", &buff[0].predraw_time);
                plot("update",  &buff[0].update_time);
            }

            ImPlot::PopStyleVar();
            ImPlot::EndPlot();
        }
    }

    ImGui::Text("| FPS:%4d |", clock.getAvgFPS());
    ImGui::SameLine();
    ImGui::Text("Tick#:%6d | ", clock.getTickCount());

    ImGui::Separator();

    if (ImGui::Checkbox("Freeze", &pauseUpdate)) {
        if (!pauseUpdate)
            unfreeze();
        else
            freeze();
    }
    ImGui::SameLine();
    if (ImGui::Button("Step")) {
        freezeStepOnce();
    }

#if not defined(__EMSCRIPTEN__)
    if (ImGui::Checkbox("Vsync Enabled", &settings.vsyncEnabled)) {
        window->setVsyncEnabled(settings.vsyncEnabled);
    }
#endif

    int fps = clock.getTargetFPS();
    constexpr std::string_view show_inf = "UNLIMITED";
    constexpr std::string_view show_fps = "%d";

#if not defined(__EMSCRIPTEN__)
    if (ImGui::DragInt("FPS", &fps, 5, 0, 500, (fps == 0 ? show_inf : show_fps).data())) {
        settings.refreshRate = fps;
        clock.setTargetFPS(fps);
    }
#endif

    int ups = clock.getTargetUPS();
    if (ImGui::DragInt("UPS", &ups, 5, 10, 500, "%d")) {
        clock.setTargetUPS(ups);
    }
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
void DebugDrawImgui::ImGui_getContent() {

    constexpr std::string_view names[] = {
        "NONE",
        "COLLISION_COLLIDER",
        "COLLISION_COLLIDABLE",
        "COLLISION_CONTACT",
        "COLLISION_RAYCAST",
        "TILELAYER_AREA",
        "TILELAYER_CHUNK",
        "CAMERA_VISIBLE",
        "CAMERA_TARGET",
        "TRIGGER_AREA",
    };
    static_assert((sizeof(names) / sizeof(names[0])) == static_cast<unsigned>(debug_draw::Type::LAST), "fix me");

    for (int i = 0; i < static_cast<unsigned>(debug_draw::Type::LAST); i++) {
        debug_draw::Type type = static_cast<debug_draw::Type>(i);
        bool draw = debug_draw::hasTypeEnabled(type);
        if (ImGui::Checkbox(names[i].data(), &draw)) {
            debug_draw::setTypeEnabled(type, draw);
        }
    }
}


}
