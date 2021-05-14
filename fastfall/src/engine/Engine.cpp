
#include <queue>
#include <memory>
#include <thread>
#include <algorithm>
#include <atomic>

#include "fastfall/engine/imgui/ImGuiFrame.hpp"

#include "fastfall/util/log.hpp"

#include "fastfall/render.hpp"

#include "fastfall/engine/Engine.hpp"
#include "fastfall/engine/config.hpp"

#include "fastfall/engine/input.hpp"

#include <iostream>


#include "fastfall/resource/Resources.hpp"
#include "fastfall/game/gfx/AnimatedSprite.hpp"

namespace ff {

unsigned getDisplayRefreshRate(const Window& win);

Engine* Engine::engineInstance = nullptr;

void Engine::init(std::unique_ptr<Window>&& window, EngineRunnable&& toRun, const Vec2u& initWindowSize, EngineSettings engineSettings) {
    LOG_INFO("Initializing engine instance");
    engineInstance = new Engine{ std::move(window), std::move(toRun), initWindowSize, engineSettings };

    LOG_INFO("Initialization complete");
}

void Engine::shutdown() {
    LOG_INFO("Shutting down engine instance");
    delete engineInstance;
    engineInstance = nullptr;

    LOG_INFO("Shutdown complete, have a nice day");
}

Engine::Engine(
    std::unique_ptr<Window>&& window,
    EngineRunnable&& toRun,
    const Vec2u& initWindowSize,
    EngineSettings engineSettings) :

    window{std::move(window)},

    initWinSize(initWindowSize),
    settings(engineSettings),
    clock(engineSettings.refreshRate), // set default FPS

    deltaTime(0.0),
    elapsedTime(0.0),
    windowZoom(1),

    ImGuiContent(ImGuiContentType::SIDEBAR_LEFT, "Engine", "System")
{

    // use first runnable to determine if we need a window
    addRunnable(std::move(toRun));
    //windowless = runnables.back().getRTexture() != nullptr;

    stepUpdate = false;
    pauseUpdate = false;
    maxDeltaTime = secs(1.0 / 60.0);
        
    initRenderTarget(false);

    initialized = true;
    if (runnables.empty()) {
        initialized = false;
    }
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

bool Engine::run()
{
    //Barrier bar(2);
    std::barrier<> bar{ 2 };

    ImGui_addContent();
    // TODO
    //instanceObs.ImGui_addContent();
    input.ImGui_addContent();

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

    if (window) {
        window->setActive();
    }

    std::thread stateWorker(&Engine::runUpdate, this, &bar);

    std::chrono::time_point<std::chrono::steady_clock> displayStart;

    running = true;

    AnimID runID = Resources::get_animation_id("player", "running");

    AnimatedSprite asprite;

    asprite.set_anim(runID);


    clock.reset();
    while (isRunning() && !runnables.empty()) {
        elapsedTime = clock.tick();

        if (window) {
            bool resetTimers = false;
            handleEvents(&resetTimers);
            if (resetTimers) {
                clock.tick();
            }

        }
        log::set_tick(clock.getTickCount());

        //update deltatime
        deltaTime = std::min(elapsedTime, maxDeltaTime);
        if (deltaTime > 0.0) {
            if (pauseUpdate && !stepUpdate) {
                deltaTime = 0.0;
            }
            stepUpdate = false;
        }

        // TODO
        Input::update(deltaTime);

        bar.arrive_and_wait();

        if (window) {
            View v = window->getView();
            EngineState* st = runnables.front().getStateHandle().getActiveState();
            Vec2f vPos = Vec2f(st->getViewPos());
            float vZoom = st->getViewZoom();

            v.setCenter(vPos);
            v.setSize(GAME_W_F * vZoom, GAME_H_F * vZoom);
            window->setView(v);
        }

        asprite.update(deltaTime);
        asprite.predraw(deltaTime);

        //draw current state
        for (auto& run : runnables) {
            drawRunnable(run);
        }
        window->draw(asprite);

        // test
        /*
        window->draw(
            ShapeCircle{
                {0.f, 0.f},
                16.f,
                32,
                ff::Color::Red
            }, ShaderProgram::getDefaultProgram()
        );
        */

        //do ImGui
        
        
        bar.arrive_and_wait();

#ifdef DEBUG
        if (window) {
            ff::ImGuiNewFrame(*window);
            ImGuiFrame::getInstance().display();
            ff::ImGuiRender();
        }
#endif

        /*
        for (auto& run : runnables) {
            if (auto rtex = run.getRTexture()) {
                // TODO
                //rtex->display();
            }
        }
        */

        bar.arrive_and_wait();


        clock.sleepUntilTick();
        if (window) {
            displayStart = std::chrono::steady_clock::now();
            window->display();
            displayTime = std::chrono::steady_clock::now() - displayStart;
        }
    }

    // clean up
    close();

    stateWorker.join();

    LOG_INFO("Display thread shutting down");

    /*
#ifdef DEBUG
    if (window) {
        //ImGui::SFML::Shutdown();
    }
#endif
    */

    running = false;

    // destroy window
    window.reset();

    return true;
}

void Engine::close() {
    if (window) {
        window->showWindow(false);
    }
    Input::closeJoystick();
    running = false;
}


void Engine::runUpdate(std::barrier<>* bar) {

    while (isRunning() && !runnables.empty()) {

        bar->arrive_and_wait();

        // do update
        for (auto& run : runnables) {
            run.getStateHandle().getActiveState()->update(deltaTime);
        }

        bar->arrive_and_wait();

        for (auto& run : runnables) {
            run.getStateHandle().getActiveState()->predraw(deltaTime);
        }

        for (auto& run : runnables) {
            run.getStateHandle().update();
        }

        bar->arrive_and_wait();


        // clean up finished runnables
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


    LOG_INFO("Update thread shutting down");

}

void Engine::handleEvents(
    // sf::Window* window, 
    bool* timeWasted)
{

    // TODO

    // no window to handle inputs from
    if (window == nullptr)
        return;

    bool discardMousePress = false;


    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // TODO
        switch (event.type) {
        case SDL_WINDOWEVENT:
            switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                close();
                break;
            case SDL_WINDOWEVENT_RESIZED:
                //ff::View view = window.getView();
                //view.setViewport({ 0, 0, event.window.data1, event.window.data2 });
                //window.setView(view);
                //break;

                Vec2i size = window->getSize();
                if (!settings.fullscreen) {
                    resizeWindow(Vec2u(size.x, size.y));
                    *timeWasted = true;
                }
                break;
            }
            break;
        case SDL_KEYUP:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                if (!settings.fullscreen) {
                    close();
                }
                else {
                    setFullscreen(!settings.fullscreen);
                }
            [[fallthrough]];
        default:
            if (!discardMousePress || (event.type != SDL_MOUSEBUTTONDOWN && event.type != SDL_MOUSEBUTTONUP))
                Input::pushEvent(event);
            break;
        }
    }


    /*


    sf::Event event;
    while (window->pollEvent(event))
    {
#ifdef DEBUG
        ImGui::SFML::ProcessEvent(event);
#endif

        switch (event.type) {

            // Window has lost focus
        case sf::Event::EventType::LostFocus:
            hasFocus = false;
            Input::resetState();
            break;

            // Window has gained focus
        case sf::Event::EventType::GainedFocus:
            hasFocus = true;
            *timeWasted = true;
            discardMousePress = true;
            Input::resetState();
            break;

            // "close requested" event: we close the window
        case sf::Event::Closed:
            close();
            *timeWasted = true;
            break;

        case sf::Event::EventType::Resized:
            if (!settings.fullscreen) {
                resizeWindow(sf::Vector2u(event.size.width, event.size.height));
                *timeWasted = true;
            }
            break;

        case sf::Event::EventType::KeyPressed:
            switch (event.key.code) {
            case sf::Keyboard::F10: setFullscreen(!settings.fullscreen); break;
            case sf::Keyboard::Escape: (settings.fullscreen ? setFullscreen(!settings.fullscreen) : running = false); break;
            case sf::Keyboard::Num1: isFrozen() ? unfreeze() : freeze(); break;
            case sf::Keyboard::Num2: freezeStepOnce(); break;
            }
            [[fallthrough]];

        default:
            if (!discardMousePress || event.type != sf::Event::EventType::MouseButtonPressed)
                Input::pushEvent(event);
            break;
        }
    }

    if (window) {
        Vec2i pixel_pos = Input::getMouseWindowPosition();
        Vec2f world_pos = Vec2f{ window->mapPixelToCoords(pixel_pos) };

        //const auto& viewport = window->getView().getViewport();
        const auto& size = window->getSize();
        bool inside = window->getView().getViewport().contains(Vec2f((float)pixel_pos.x / size.x, (float)pixel_pos.y / size.y));

        Input::setMouseWorldPosition(world_pos);
        Input::setMouseInView(inside);

        //LOG_INFO("Viewport: t{}, l{}, w{}, h{}", viewport.top, viewport.left, viewport.width, viewport.height);
        //LOG_INFO("Mouse: {:4.2f}, {:4.2f}, {}", world_pos.x, world_pos.y, inside);
    }
    */
}

void Engine::initRenderTarget(bool fullscreen)
{
    settings.fullscreen = fullscreen;

    SDL_DisplayMode mode;
    int r = SDL_GetCurrentDisplayMode(0, &mode);
    assert(r == 0);

    Vec2u displaySize(mode.w, mode.h);

    char version[32];
    sprintf_s(version, "%d.%d.%d", VERSION[0], VERSION[1], VERSION[2]);

    window->setWindowTitle(fmt::format("GamePro2 v{}", version));
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
    window->setVsyncEnabled(settings.vsyncEnabled);
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

    char version[32];
    sprintf_s(version, "%d.%d.%d", VERSION[0], VERSION[1], VERSION[2]);

    SDL_DisplayMode mode;
    int r = SDL_GetCurrentDisplayMode(0, &mode);
    assert(r == 0);

    //window->showWindow(false);

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
    margins->glTransfer();

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


    ImGui::BulletText("Window");
    ImGui::Text("Window Pos   = (%4d, %4d)", window->getPosition().x, window->getPosition().y);
    ImGui::Text("Window Size  = (%4d, %4d)", window->getSize().x, window->getSize().y);
    bool goFullscreen = settings.fullscreen;
    if (ImGui::Checkbox("Fullscreen", &goFullscreen)) {
        setFullscreen(goFullscreen);
    }
    ImGui::Separator();
    glm::fvec4 vp = window->getView().getViewport();
    ImGui::Text("Viewport     = (%6.2f, %6.2f, %6.2f, %6.2f)", vp[0], vp[1], vp[2], vp[3]);
    ImGui::Text("Zoom         =  %2dx", windowZoom);
    ImGui::Separator();


    // FRAME DATA GRAPH

    static constexpr int arrsize = 301;
    static constexpr int last = arrsize - 1;

    static float active_x [arrsize] = { 0.f };
    static float active_y [arrsize] = { 0.f };
    static float sleep_x  [arrsize] = { 0.f };
    static float sleep_y  [arrsize] = { 0.f };
    static float display_x[arrsize] = { 0.f };
    static float display_y[arrsize] = { 0.f };

    static bool initX = false;
    if (!initX) {
        for (int x = 0; x <= last; x++) {
            active_x [x] = static_cast<float>(x);
            sleep_x  [x] = static_cast<float>(x);
            display_x[x] = static_cast<float>(x);
        }
        initX = true;
    }

    //double denom = (clock.getTargetFPS() != 0 ? clock.getTickDuration() * 2000.f : 2.0);
    float denom = 1000.0 / clock.getAvgFPS();

    float tick_x[2] = { 0.0, (arrsize - 1) };
    float tick_y[2] = { denom, denom };

    memmove(&active_y[0], &active_y[1], sizeof(float) * last);
    memmove(&sleep_y[0], &sleep_y[1], sizeof(float) * last);
    memmove(&display_y[0], &display_y[1], sizeof(float) * last);
    
    float acc = 0.f;
    acc += clock.data().activeTime.count() * 1000.0;
    active_y [last] = acc;
    acc += clock.data().sleepTime.count() * 1000.0;
    sleep_y  [last] = acc;
    acc += displayTime.count() * 1000.0;
    display_y[last] = acc;

    if (ImGui::CollapsingHeader("Framerate Graph", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImPlot::SetNextPlotLimits(0.0, (arrsize - 1), 0.0, denom * 2.f, ImGuiCond_Always);
        if (ImPlot::BeginPlot("##FPS", NULL, "tick time (ms)", ImVec2(-1, 200), ImPlotFlags_None, ImPlotAxisFlags_None, 0)) {
            ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 1.f);

            ImPlot::PlotShaded("display", display_x, display_y, arrsize);
            ImPlot::PlotShaded("sleep", sleep_x, sleep_y, arrsize);
            ImPlot::PlotShaded("active", active_x, active_y, arrsize);

            ImPlot::PopStyleVar();

            if (clock.getTargetFPS() != 0) {
                ImPlot::PlotLine("tick time", tick_x, tick_y, 2);
            }

            ImPlot::EndPlot();
        }

    }

    // END FRAME DATA GRAPH

    ImGui::Text("| FPS: %4d |", clock.getAvgFPS());
    ImGui::SameLine();
    ImGui::Text("Gamespeed: %3.0f%% | ", 100.0 * (elapsedTime > maxDeltaTime ? maxDeltaTime / elapsedTime : 1.f));
    ImGui::SameLine();
    ImGui::Text("Tick#: %8d | ", clock.data().tickTotal);
    ImGui::SameLine();
    ImGui::Text("Tick Miss: %2d | ", clock.data().tickMissPerSec);

    ImGui::Separator();

    static bool fpsUnlimited = false;
    static int fpsMax = settings.refreshRate;
    static bool steady = true;
    //static bool vsync = false;

    if (ImGui::Checkbox("Freeze", &pauseUpdate)) {
        if (!pauseUpdate)
            log::set_verbosity(log::level::VERB);
        else
            log::set_verbosity(log::level::STEP);
    }
    ImGui::SameLine();
    if (ImGui::Button("Step")) {
        pauseUpdate = true;
        stepUpdate = true;
        LOG_STEP("STEP");
    }

    if (ImGui::Checkbox("Set FPS Unlimited", &fpsUnlimited)) {
        settings.refreshRate = fpsUnlimited ? 0 : fpsMax;
        clock.setTargetFPS(settings.refreshRate);
    }
    if (ImGui::Checkbox("Set Steady Tickrate", &steady)) {
        clock.setSteady(steady);
    }
    if (ImGui::Checkbox("Set Vsync", &settings.vsyncEnabled)) {
        if (settings.vsyncEnabled) {
            int vsyncRefreshRate = getDisplayRefreshRate(*window);
            if (vsyncRefreshRate > 0) {
                fpsMax = vsyncRefreshRate;
                clock.setTargetFPS(vsyncRefreshRate);
            }
        }
        else {
            clock.setTargetFPS(settings.refreshRate);
        }
        window->setVsyncEnabled(settings.vsyncEnabled);
    }

    if (ImGui::DragInt("Set Frame Limit", &fpsMax, 5, 10, 500, "%.0f")) {
        settings.refreshRate = fpsUnlimited ? 0 : fpsMax;
        clock.setTargetFPS(settings.refreshRate);
    }

}

void Engine::ImGui_getExtraContent() {
    if (showImGuiDemo) ImGui::ShowDemoWindow(&showImGuiDemo);
    if (showImPlotDemo) ImPlot::ShowDemoWindow(&showImPlotDemo);
}

unsigned getDisplayRefreshRate(const Window& win) {

    SDL_DisplayMode mode;
    int displayIndex = SDL_GetWindowDisplayIndex(win.getSDL_Window());

    int defaultRefreshRate = 60;
    if (SDL_GetDesktopDisplayMode(displayIndex, &mode) != 0)
    {
        return defaultRefreshRate;
    }
    if (mode.refresh_rate == 0)
    {
        return defaultRefreshRate;
    }
    return mode.refresh_rate;

}


}