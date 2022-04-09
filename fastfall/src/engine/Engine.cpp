
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
#endif


namespace ff {

std::optional<unsigned> getDisplayRefreshRate(const Window& win);

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
    EngineSettings engineSettings) :

    window{std::move(initWindow)},

    initWinSize(initWindowSize),
    settings(engineSettings),
    clock(), // set default FPS

    //deltaTime(0.0),
    elapsedTime(0.0),
    windowZoom(1),

    ImGuiContent(ImGuiContentType::SIDEBAR_LEFT, "Engine", "System")
{

#if defined(__EMSCRIPTEN__)
    clock.setSteady(false);
#endif

    // use first runnable to determine if we need a window
    addRunnable(std::move(toRun));
    //windowless = runnables.back().getRTexture() != nullptr;

    stepUpdate = false;
    pauseUpdate = false;
    //maxDeltaTime = secs{ 1.0 / 30.0 };

    initRenderTarget(false);

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

    while (isRunning() && !runnables.empty()) {


        updateTimer();

        Input::update(elapsedTime);

        while (update_counter > 0) {
            updateRunnables();
            update_counter--;
        }

        predrawRunnables();

        updateView();

        drawRunnables();

        updateImGui();

        updateStateHandler();

        cleanRunnables();

        ff::glDeleteStale();

        sleep();
    }

    // clean up
    close();

    LOG_INFO("Run thread shutting down");

    running = false;

    ff::glDeleteStale();

    // destroy window
    //window.reset();

    return true;
}


// -------------------------------------------

bool Engine::run_doubleThread()
{

    prerun_init();
    SDL_SetThreadPriority(SDL_ThreadPriority::SDL_THREAD_PRIORITY_HIGH);


    std::barrier<> bar{ 2 };

    running = true;
    clock.reset();
    std::thread stateWorker(&Engine::runUpdate, this, &bar);

    while (isRunning() && !runnables.empty()) {
        updateTimer();

        Input::update(elapsedTime);

        bar.arrive_and_wait();

        // do update/draw

        updateView();

        drawRunnables();

        bar.arrive_and_wait();

		// predraw

        updateImGui();

        //bar.arrive_and_wait();

		// clean

        ff::glDeleteStale();

        sleep();
    }

    // clean up
    close();

    LOG_INFO("Update thread shutting down");

    stateWorker.join();

    LOG_INFO("Display thread shutting down");

    running = false;

    ff::glDeleteStale();

    // destroy window
    //window.reset();

    return true;
}



void Engine::runUpdate(std::barrier<>* bar) {
    SDL_SetThreadPriority(SDL_ThreadPriority::SDL_THREAD_PRIORITY_HIGH);

    while (isRunning() && !runnables.empty()) {

        bar->arrive_and_wait();

        // update/draw
	
        updateRunnables();

        bar->arrive_and_wait();

		// predraw

        predrawRunnables();

        updateStateHandler();

        //bar->arrive_and_wait();

		// cleanup

        cleanRunnables();
    }
}

// -------------------------------------------

bool Engine::run_emscripten() {
#if defined(__EMSCRIPTEN__)

    prerun_init();

    running = true;

	emscripten_set_main_loop_arg(Engine::emscripten_loop, this, 0, false);
    //window->setVsyncEnabled(settings.vsyncEnabled);

#endif
    return true;

}

void Engine::emscripten_loop(void* engine_ptr) {
	Engine* engine = (Engine*)engine_ptr;

	if(!engine->isRunning() || engine->runnables.empty())
		return;

	engine->updateTimer();

	Input::update(engine->elapsedTime);

	engine->updateRunnables();

	engine->predrawRunnables();

	engine->updateView();

	engine->drawRunnables();

	engine->updateImGui();

	engine->updateStateHandler();

	engine->cleanRunnables();

	ff::glDeleteStale();

	engine->sleep();
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

    auto [delta, update_count, interp] = clock.tick();
    //elapsedTime = elapsed;
    elapsedTime = delta;
    update_counter = update_count;
    interpolation = interp;
    hasUpdated = false;

    //LOG_INFO("elapsed={:.5f}ms update={} interp={}", elapsedTime * 1000.0, update_counter, interpolation)

    if (window) {
        bool resetTimers = false;
        handleEvents(&resetTimers);
        if (resetTimers) {
            clock.tick();
        }

    }

    if (ResourceWatcher::is_watch_running()) {
        if (Resources::reloadOutOfDateAssets()) {
           // clock.tick();
        }
    }

   // log::set_tick(clock.getTickCount());
    log::set_tick(0);
    
    //update deltatime
    /*
    deltaTime = elapsedTime;
    if (deltaTime > 0.0) {
        if (pauseUpdate && !stepUpdate) {
            deltaTime = 0.0;
        }
        stepUpdate = false;
    }
    */

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
        Vec2f vPos = Vec2f(st->getViewPos());
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

void Engine::updateRunnables() {
    hasUpdated = true;
    for (auto& run : runnables) {
        run.getStateHandle().getActiveState()->update(clock.upsDuration());
    }
}
void Engine::predrawRunnables() {
    for (auto& run : runnables) {
        run.getStateHandle().getActiveState()->predraw(interpolation, hasUpdated);
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

void Engine::sleep() {
    if (window) {
        displayStart = std::chrono::steady_clock::now();
        window->display();
        displayTime = std::chrono::steady_clock::now() - displayStart;
    }


    //clock.sleepUntilTick(!window || settings.vsyncEnabled);

    clock.sleep();
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

    //std::string title = fmt::format("game v{}.{}.{}", VERSION[0], VERSION[1], VERSION[2]);
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
    //margins->glTransfer();

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

    // FRAME DATA GRAPH

    static constexpr int arrsize = 101;
    static constexpr int last = arrsize - 1;

    static float active_x [arrsize] = { 0.f };
    static float active_y [arrsize] = { 0.f };
    static float sleep_x  [arrsize] = { 0.f };
    static float sleep_y  [arrsize] = { 0.f };
    static float display_x[arrsize] = { 0.f };
    static float display_y[arrsize] = { 0.f };

    static bool initX = false;

    static int roller = 0;

    if (!initX) {
        for (int x = 0; x <= last; x++) {
            active_x [x] = static_cast<float>(x);
            sleep_x  [x] = static_cast<float>(x);
            display_x[x] = static_cast<float>(x);
        }
        initX = true;
    }

    //double denom = (clock.getTargetFPS() != 0 ? clock.getTickDuration() * 2000.f : 2.0);
    float denom =  1000.0 / (clock.getAvgFPS() > 0 ? clock.getAvgFPS() : 60);

    float tick_x[2] = { 0.0, (arrsize - 1) };
    float tick_y[2] = { denom, denom };

    memmove(&active_y[0], &active_y[1], sizeof(float) * last);
    memmove(&sleep_y[0], &sleep_y[1], sizeof(float) * last);
    memmove(&display_y[0], &display_y[1], sizeof(float) * last);
    
    float acc = 0.f;
    //acc += clock.data().activeTime.count() * 1000.0 - (displayTime.count() * 1000.0);
    active_y [last] = acc;
    //acc += displayTime.count() * 1000.0;
    display_y[last] = acc;
    //acc += clock.data().sleepTime.count() * 1000.0;
    sleep_y  [last] = acc;

    roller++;
    if (roller == last) {
        roller %= last;
    }


    if (ImGui::CollapsingHeader("Framerate Graph", ImGuiTreeNodeFlags_DefaultOpen)) {

        
        ImPlot::SetNextPlotLimits(0.0, (arrsize - 1), 0.0, denom * 2.f, ImGuiCond_Always);
        if (ImPlot::BeginPlot("##FPS", NULL, "tick time (ms)", ImVec2(-1, 200), ImPlotFlags_None, ImPlotAxisFlags_None, 0)) {
            ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 1.f);

            ImPlot::PlotShaded("sleep", sleep_x, sleep_y, arrsize);
            ImPlot::PlotShaded("display", display_x, display_y, arrsize);
            ImPlot::PlotShaded("active", active_x, active_y, arrsize);

            ImPlot::PopStyleVar();

            if (clock.getTargetFPS() != 0) {
                ImPlot::PlotLine("target tick", tick_x, tick_y, 2);
            }
            else {
                ImPlot::PlotLine("avg tick", tick_x, tick_y, 2);
            }

            ImPlot::EndPlot();
        }
        

    }

    // END FRAME DATA GRAPH

    ImGui::Text("| FPS:%4d |", clock.getAvgFPS());
    ImGui::SameLine();
    //ImGui::Text("Speed:%3.0f%% | ", 100.0 * (elapsedTime > maxDeltaTime ? maxDeltaTime / elapsedTime : 1.f));
    ImGui::SameLine();
    //ImGui::Text("Tick#:%6d | ", clock.data().tickTotal);
    ImGui::SameLine();
    //ImGui::Text("Tick Miss:%2d | ", clock.data().tickMissPerSec);
   // ImGui::SameLine();
    static float tickMS = 0.f;
    if (roller == 0) {
        //tickMS = clock.data().activeTime.count() * 1000.f;
    }
    ImGui::Text("Tick MS:%2.1f | ", tickMS);
    ImGui::SameLine();
    //ImGui::Text("Delta Time MS:%2.1f | ", deltaTime * 1000.0);

    ImGui::SameLine();

    ImGui::Text("Active MS:%2.1f | ", active_y[100]);

    ImGui::Separator();

    static bool fpsUnlimited = false;
    static int fpsMax = settings.refreshRate;
    static bool steady = true;
    //static bool vsync = false;

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

    if (ImGui::Checkbox("Set FPS Unlimited", &fpsUnlimited)) {
        settings.refreshRate = fpsUnlimited ? 0 : fpsMax;
        //clock.setTargetFPS(settings.refreshRate);
    }
    if (ImGui::Checkbox("Set Steady Tickrate", &steady)) {
        //clock.setSteady(steady);
    }
    if (ImGui::Checkbox("Set Vsync", &settings.vsyncEnabled)) {
        if (settings.vsyncEnabled) 
        {
            unsigned vsyncRefreshRate = getDisplayRefreshRate(*window).value_or(60);
            fpsMax = vsyncRefreshRate;
            //clock.setTargetFPS(vsyncRefreshRate);
        }
        else {
            //clock.setTargetFPS(settings.refreshRate);
        }
        window->setVsyncEnabled(settings.vsyncEnabled);
    }

    if (ImGui::DragInt("Set Frame Limit", &fpsMax, 5, 10, 500, "%.0f")) {
        settings.refreshRate = fpsUnlimited ? 0 : fpsMax;
        //clock.setTargetFPS(settings.refreshRate);
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


std::optional<unsigned> getDisplayRefreshRate(const Window& win) 
{
    SDL_DisplayMode mode;
    int displayIndex = SDL_GetWindowDisplayIndex(win.getSDL_Window());

    if ((SDL_GetDesktopDisplayMode(displayIndex, &mode) != 0) || (mode.refresh_rate == 0))
    {
        return std::nullopt;
    } 
    return mode.refresh_rate;
}


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
