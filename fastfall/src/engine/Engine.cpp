
#include <queue>
#include <memory>
#include <thread>
#include <algorithm>
#include <atomic>

//SFML
//#include <SFML/Window.hpp>

//IMGUI
/*
#include "imgui/imgui.h"
#include "implot/implot.h"
#include "imgui/misc/cpp/imgui_stdlib.h"
*/

//#include "imgui.h"
//#include "implot.h"
//#include "misc/cpp/imgui_stdlib.h"

//IMGUI-SFML
//#include "ImGui-SFML/imgui-SFML.h"

#include "fastfall/engine/imgui/ImGuiFrame.hpp"

#include "fastfall/util/log.hpp"

#include "fastfall/render.hpp"

#include "fastfall/engine/Engine.hpp"
//#include "time/EngineClock.hpp"
#include "fastfall/engine/config.hpp"

//#include "TestState.hpp"


#include <iostream>
//#include <SFML/OpenGL.hpp>

namespace ff {

unsigned getDisplayRefreshRate(const Window& win);

Engine* Engine::engineInstance = nullptr;

void Engine::init(EngineRunnable&& toRun, const Vec2u& initWindowSize, EngineSettings engineSettings) {
    LOG_INFO("Initializing engine instance");
    engineInstance = new Engine{ std::move(toRun), initWindowSize, engineSettings };

    LOG_INFO("Initialization complete");
}

void Engine::shutdown() {
    LOG_INFO("Shutting down engine instance");
    delete engineInstance;
    engineInstance = nullptr;

    LOG_INFO("Shutdown complete, have a nice day");
}



Engine::Engine(
    EngineRunnable&& toRun,
    const Vec2u& initWindowSize,
    EngineSettings engineSettings) :

    // TODO
   // margins(sf::TriangleStrip, 10),
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
    windowless = runnables.back().getRTexture() != nullptr;

    stepUpdate = false;
    pauseUpdate = false;
    maxDeltaTime = secs(1.0 / 60.0);

    // TODO
    /*
    for (int i = 0; i < 10; i++) {
        margins[i].color = Color::Black;
    }
    */

    if (!windowless) {
        initRenderTarget(false);
    }

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
        // TODO
        // target->draw(margins);
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
    //input.ImGui_addContent();

#ifdef DEBUG
    if (window != nullptr) {
       // ImGui::SFML::Init(*window);
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

    running = true;

    clock.reset();
    while (isRunning() && !runnables.empty()) {
        elapsedTime = clock.tick();

        if (window) {
            bool resetTimers = false;
            handleEvents(/*window.get(),*/ &resetTimers);
            if (resetTimers) {
                //clock.reset();
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
        //Input::update(deltaTime);

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

        //draw current state
        for (auto& run : runnables) {
            drawRunnable(run);
        }

        bar.arrive_and_wait();

        //do ImGui
#ifdef DEBUG
        if (window) {
            // TODO

            ff::ImGuiNewFrame(*window);
           // ImGui::SFML::Update(*window, sf::seconds(static_cast<float>(elapsedTime)));
            //updateImGUI();

            ImGuiFrame::getInstance().display();

            ff::ImGuiRender();
        }
#endif

        for (auto& run : runnables) {
            if (auto rtex = run.getRTexture()) {
                // TODO
                //rtex->display();
            }
        }

        //clock.sleepUntilTick(vsyncEnabled);
        bar.arrive_and_wait();

        clock.sleepUntilTick();
        if (window) {
            window->display();
        }
    }

    // clean up
    close();

    stateWorker.join();

    LOG_INFO("Display thread shutting down");

#ifdef DEBUG
    if (window) {
        //ImGui::SFML::Shutdown();
    }
#endif

    running = false;

    // destroy window
    window.reset();

    return true;
}

void Engine::close() {
    //TODO
    /*
    if (window) {
        window->close();
    }
    */

    if (window) {
        window->showWindow(false);
    }

    //signals.bail = true;
    running = false;
}


void Engine::runUpdate(std::barrier<>* bar) {

    // sf::VertexArray* margin = &margins;
        //sf::View* mv = &marginView;

    // secf tBuffer = 0us;
    while (isRunning() && !runnables.empty()) {

        bar->arrive_and_wait();

        // do update
        for (auto& run : runnables) {
            run.getStateHandle().getActiveState()->update(deltaTime);
        }

        //LOG_INFO("tick");

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
        case SDL_KEYUP:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                close();
            break;
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

    //sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    //Vec2u displaySize(desktop.width, desktop.height);
    Vec2u displaySize(mode.w, mode.h);

    char version[32];
    sprintf_s(version, "%d.%d.%d", VERSION[0], VERSION[1], VERSION[2]);

    if (settings.fullscreen) {
        // go borderless fullscreen

        /*
        window = std::make_unique<Window>(
            // displaySize.y + 1 fixes borderless? https://github.com/SFML/SFML/issues/1284
            sf::VideoMode(displaySize.x, displaySize.y + 1, 32),
            "GamePro2 v" + std::string(version),
            sf::Style::None);
        */
        window = std::make_unique<Window>(fmt::format("GamePro2 v{}", version), displaySize.x, displaySize.y);

    }
    else {
        /*
        window = std::make_unique<sf::RenderWindow>(
            sf::VideoMode(initWinSize.x, initWinSize.y),
            "GamePro2 v" + std::string(version),
            sf::Style::Default);

        window->setPosition(sf::Vector2i(
            (displaySize.x - initWinSize.x) / 2,
            (displaySize.y - initWinSize.y) / 2));
        */
        window = std::make_unique<Window>(fmt::format("GamePro2 v{}", version), 
            initWinSize.x,
            initWinSize.y
        );
        window->setWindowCentered();
        window->setWindowResizable();

    }

   // window->setVerticalSyncEnabled(settings.vsyncEnabled);
    //window->setKeyRepeatEnabled(false);

    window->setActive();
    window->setVsyncEnabled(settings.vsyncEnabled);
    window->showWindow();

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

        //sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        //sf::Vector2u displaySize(desktop.width, desktop.height);

        Vec2i displaySize{mode.w, mode.h};

        // go borderless fullscreen
        /*
        window->create(
            // displaySize.y + 1 fixes borderless? https://github.com/SFML/SFML/issues/1284
            sf::VideoMode(displaySize.x, displaySize.y + 1, 32),
            "GamePro2 v" + std::string(version),
            sf::Style::None

        );
        */

        //window = std::make_unique<Window>(fmt::format("GamePro2 v{}", version), displaySize.x, displaySize.y);

        SDL_SetWindowFullscreen(window->getSDL_Window(), SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    else {
        // go windowed
        /*
        window->create(
            sf::VideoMode(lastWindowedSize.x, lastWindowedSize.y, 32),
            "GamePro2 v" + std::string(version),
            sf::Style::Default
        );
        window->setPosition(lastWindowedPos);
        */

        //window = std::make_unique<Window>(fmt::format("GamePro2 v{}", version), lastWindowedSize.x, lastWindowedSize.y);
        SDL_SetWindowFullscreen(window->getSDL_Window(), 0);
        window->setWindowPosition(lastWindowedPos);
        window->setWindowResizable();
    }

    //window->setVerticalSyncEnabled(settings.vsyncEnabled);
    //window->setKeyRepeatEnabled(false);

    //resizeWindow(window->getSize());
    window->setActive();
    window->setVsyncEnabled(settings.vsyncEnabled);
   // window->showWindow();

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
        (float)offX / finalSize.x,
        (float)offY / finalSize.y,
        (float)trueX / finalSize.x,
        (float)trueY / finalSize.y
    });

    v.setSize(GAME_W_F, GAME_H_F);
    //v.setCenter(GAME_W / 2, GAME_H / 2);
    window->setView(v);

    windowZoom = scale;

    //update margins
    // 
    // TODO
    
    glm::fvec4 vp = window->getView().getViewport();

    Recti viewport{ 
        (int)(vp[0] * window->getSize().x), 
        (int)(vp[1] * window->getSize().y),
        (int)(vp[2] * window->getSize().x),
        (int)(vp[3] * window->getSize().y)
    };
    Recti windowSize(0, 0, window->getSize().x, window->getSize().y);

    /*
    margins[0].position = sf::Vector2f(static_cast<float>(windowSize.left),
        static_cast<float>(windowSize.top));
    margins[1].position = sf::Vector2f(static_cast<float>(viewport.left),
        static_cast<float>(viewport.top));
    margins[2].position = sf::Vector2f(static_cast<float>(windowSize.left),
        static_cast<float>(windowSize.top + windowSize.height));
    margins[3].position = sf::Vector2f(static_cast<float>(viewport.left),
        static_cast<float>(viewport.top + viewport.height));
    margins[4].position = sf::Vector2f(static_cast<float>(windowSize.left + windowSize.width),
        static_cast<float>(windowSize.top + windowSize.height));
    margins[5].position = sf::Vector2f(static_cast<float>(viewport.left + viewport.width),
        static_cast<float>(viewport.top + viewport.height));
    margins[6].position = sf::Vector2f(static_cast<float>(windowSize.left + windowSize.width),
        static_cast<float>(windowSize.top));
    margins[7].position = sf::Vector2f(static_cast<float>(viewport.left + viewport.width),
        static_cast<float>(viewport.top));
    margins[8].position = sf::Vector2f(static_cast<float>(windowSize.left),
        static_cast<float>(windowSize.top));
    margins[9].position = sf::Vector2f(static_cast<float>(viewport.left),
        static_cast<float>(viewport.top));

    marginView = sf::View(sf::FloatRect(
        0.f,
        0.f,
        static_cast<float>(window->getSize().x),
        static_cast<float>(window->getSize().y)
    ));
    */

    //TODO
    
    if (ImGuiFrame::getInstance().isDisplay()) {
        ImGuiFrame::getInstance().resize(windowSize, Vec2u(viewport.getSize().x, viewport.getSize().y));
    }
    
}

bool showImGuiDemo = false;
bool showImPlotDemo = false;

void Engine::ImGui_getContent() {
    //static bool showImGuiDemo = false;
    //static bool showImPlotDemo = false;

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
    // TODO
    //sf::FloatRect vp = window->getView().getViewport();
    //ImGui::Text("Viewport     = (%6.2f, %6.2f, %6.2f, %6.2f)", vp.left, vp.top, vp.width, vp.height);
    ImGui::Text("Zoom         =  %2dx", windowZoom);
    ImGui::Separator();

    ImGui::BulletText("Frame Data");


    static constexpr int arrsize = 301;
    //static constexpr int arrsizeminus1 = 300;
    //int cur = clock.data().tickTotal % arrsize;

    static ImPlotPoint active[arrsize];
    static ImPlotPoint sleep[arrsize];
    //static ImPlotPoint display[arrsize];
    double denom = (clock.getTargetFPS() != 0 ? clock.getTickDuration() : 1.0);
    ImPlotPoint tick[2] = {
        ImPlotPoint(0.0,           denom * 1000.0),
        ImPlotPoint((arrsize - 1), denom * 1000.0)
    };

    memmove(&active[0], &active[1], sizeof(ImPlotPoint) * (arrsize - 1));
    memmove(&sleep[0], &sleep[1], sizeof(ImPlotPoint) * (arrsize - 1));
    //memmove(&display[0], &display[1], sizeof(ImPlotPoint) * arrsizeminus1);

    for (int i = 0; i < (arrsize - 1); i++) {
        active[i].x -= 1.0;
        sleep[i].x -= 1.0;
        //display[i].x -= 1.0;
    }

    active[(arrsize - 1)].x = static_cast<double>((arrsize - 1));
    active[(arrsize - 1)].y = clock.data().activeTime.count() * 1000.0;// / denom;

    sleep[(arrsize - 1)].x = static_cast<double>((arrsize - 1));
    sleep[(arrsize - 1)].y = active[(arrsize - 1)].y + clock.data().sleepTime.count() * 1000.0;

    //display[arrsizeminus1].x = static_cast<double>(arrsizeminus1);
    //display[arrsizeminus1].y = sleep[arrsizeminus1].y + display_duration * 1000.0;// / denom;


    ImPlot::SetNextPlotLimits(0.0, (arrsize - 1), 0.0, denom * 2000.0, ImGuiCond_Always);
    if (ImPlot::BeginPlot("##FPS", NULL, NULL, ImVec2(-1, 150), ImPlotFlags_None, ImPlotAxisFlags_None, 0)) {
        ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.75f);
        //ImPlot::PlotShaded("display time", &display[0].x, &display[0].y, arrsize, 0, 0, 16);
        ImPlot::PlotShaded("sleep time", &sleep[0].x, &sleep[0].y, arrsize, 0, 0, 16);
        ImPlot::PlotShaded("active time", &active[0].x, &active[0].y, arrsize, 0, 0, 16);
        ImPlot::PopStyleVar();
        //ImPlot::PlotLine("tick time", tick, 2, 0);

        ImPlot::EndPlot();
    }

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

// OS-specific stuff

#if defined(_WIN32)

#include <windows.h>

unsigned getDisplayRefreshRate(const Window& win) {

    SDL_DisplayMode Mode;
    int DisplayIndex = SDL_GetWindowDisplayIndex(win.getSDL_Window());
    // If we can't find the refresh rate, we'll return this:
    int DefaultRefreshRate = 60;
    if (SDL_GetDesktopDisplayMode(DisplayIndex, &Mode) != 0)
    {
        return DefaultRefreshRate;
    }
    if (Mode.refresh_rate == 0)
    {
        return DefaultRefreshRate;
    }
    return Mode.refresh_rate;


    /*
    HWND whdl = win.getSystemHandle();
    HMONITOR mon = MonitorFromWindow(whdl, MONITOR_DEFAULTTOPRIMARY);

    MONITORINFOEX info;
    info.cbSize = sizeof(MONITORINFOEX);

    DEVMODEA devmode;
    devmode.dmSize = sizeof(DEVMODEA);

    if (GetMonitorInfoA(mon, &info) && EnumDisplaySettingsA(info.szDevice, ENUM_CURRENT_SETTINGS, &devmode)) {
        return devmode.dmDisplayFrequency;
    }
    else {
        return 0;
    }
    */
}

#elif defined(unix)
// TODO
static_assert(false, "getDisplayRefreshRate isn't defined for this platform!");
#elif defined(__APPLE__)
// TODO
static_assert(false, "getDisplayRefreshRate isn't defined for this platform!");
#else

static_assert(false, "getDisplayRefreshRate isn't defined for this platform!");
#endif

}