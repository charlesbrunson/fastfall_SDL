#pragma once

#include "time/FixedEngineClock.hpp"
#include "state/EngineStateHandler.hpp"
#include "EngineRunnable.hpp"

#include "imgui/ImGuiContent.hpp"

#include "fastfall/util/Vec2.hpp"

#include "fastfall/render/drawable/VertexArray.hpp"
#include "fastfall/render/target/Window.hpp"
#include "fastfall/engine/input/InputConfig.hpp"
#include "fastfall/game/WorldImGui.hpp"

#include <queue>
#include <memory>
#include <mutex>
#include <functional>
#include <chrono>

#include <mutex>
#include <barrier>

namespace ff {

constexpr int VERSION[3] = { 0, 0, 2 };

enum class EngineRunStyle {
	SingleThread,
	DoubleThread,
	//Emscripten
};

struct EngineSettings {
	// window margins
	bool allowMargins = true;

	// display refresh rate
	int refreshRate = 0;
	bool vsyncEnabled = false;

	// display fullscreen
	bool fullscreen = false;

	// display ImGui
	bool showDebug = false;

	// run style
	EngineRunStyle runstyle = EngineRunStyle::DoubleThread;
};

class DebugDrawImgui : public ImGuiContent {
public:
	DebugDrawImgui();
	void ImGui_getContent(secs deltaTime) override;
};


class Engine : public ImGuiContent {
	DebugDrawImgui debugdrawImgui;
	InputConfig::InputObserver input_cfg;
    WorldImGui worldImgui;

	RenderTexture target;

public:
    explicit Engine(Window* window);
    Engine(Window* window, const Vec2u init_window_size, EngineSettings settings);

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    Engine(Engine&&) = default;
    Engine& operator=(Engine&&) = default;

	~Engine() override;

    void push_runnable(EngineRunnable&& toRun);

    template<typename InitState>
    void make_runnable(auto&&... args) {
        push_runnable(EngineRunnable{
            std::make_unique<InitState>( std::forward<decltype(args)>(args)... ),
            false
        });
    }

    bool run();

    bool is_running() const noexcept;
    bool is_init() const noexcept;

    int get_window_scale() const noexcept;
    const Window* get_window() const noexcept;
    const FixedEngineClock& get_clock() const;
    secs get_uptime() const;

    void resizeWindow(Vec2u size, bool force_size = false);
private:
	void freeze();
	void freezeStepOnce();
	void unfreeze();
	bool isFrozen() const noexcept;

	// run strategies
	bool run_doubleThread();
	bool run_singleThread();
	bool run_emscripten();
	static void emscripten_loop(void* engine_ptr);

	void prerun_init();

	// engine loop
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
};

}
