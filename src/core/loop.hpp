#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include <functional>
#include <chrono>

#include <mutex>
#include <barrier>

namespace ff {
namespace loop {

/*
    void push_runnable(EngineRunnable&& toRun);

    template<typename InitState>
    void make_runnable(auto&&... args) {
        push_runnable(EngineRunnable{
            std::make_unique<InitState>( std::forward<decltype(args)>(args)... ),
            false
        });
    }
*/

    bool run();

    bool is_running();
    bool is_init();

/*
    int get_window_scale() const noexcept;
    const Window* get_window() const noexcept;
    const FixedEngineClock& get_clock() const;
    secs get_uptime() const;

    void resizeWindow(Vec2u size, bool force_size = false);
*/

    void freeze();
    void step();
    void unfreeze();
    bool is_frozen();

    // run strategies
/*
    bool run_doubleThread();
    bool run_singleThread();
    bool run_emscripten();
    static void emscripten_loop(void* engine_ptr);
*/

//    void prerun_init();

    // engine loop
/*
    void updateTimer();
    void updateStateHandler();
    void updateView();
    void updateRunnables();
    void predrawRunnables();
    void drawRunnables();
    void updateImGui();
    void cleanRunnables();
    void display();
    void sleep();
*/

/*
    EngineSettings settings;

    bool initialized = false;

    // thread-sync helper
    bool running = false;

    std::optional<float> next_timescale;

    unsigned int avgFPS = 0;
    unsigned int avgUPS = 0;

    // display management
    Window* window = nullptr;
    Vec2i lastWindowedPos;
    Vec2u initWinSize;
    Vec2u lastWindowedSize;
    int windowZoom = 1;

    bool wantFullscreen = false;
    bool hasFocus = true;

    // margins
    std::unique_ptr<VertexArray> margins;
    View marginView;

    // framerate & time management
    FixedEngineClock clock;
    FixedEngineClock::Tick tick;
    bool hasUpdated = false;

    secs upTime = 0.0;

    // event handling
    unsigned event_count = 0u;

    bool pauseUpdate = false;
    bool stepUpdate  = false;
    bool interpolate = true;

    std::vector<EngineRunnable> runnables;

    void initRenderTarget(bool fullscreen);
    void runUpdate(std::barrier<>* bar);
    void drawRunnable(EngineRunnable& run);

    void close();

    void handleEvents(bool* timeWasted);
    bool setFullscreen(bool fullscreen);

    void ImGui_getContent(secs deltaTime) override;
    void ImGui_getExtraContent() override;
*/
}
}
