#pragma once

#include "time/EngineClock.hpp"
#include "time/FixedEngineClock.hpp"
#include "state/EngineStateHandler.hpp"
#include "EngineRunnable.hpp"

#include "imgui/ImGuiContent.hpp"

#include "fastfall/util/Vec2.hpp"

#include "fastfall/render/VertexArray.hpp"
#include "fastfall/render/Window.hpp"
#include "fastfall/engine/input.hpp"

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
	Emscripten
};

struct EngineSettings {
	// window margins
	bool allowMargins = true;

	// display refresh rate
	int refreshRate = 60;
	bool vsyncEnabled = false;

	// display fullscreen
	bool fullscreen = false;

	// display ImGui
	bool showDebug = false;

	// run displayless
	bool noWindow = false;

	// run style
	EngineRunStyle runstyle = EngineRunStyle::DoubleThread;
};

class DebugDrawImgui : public ImGuiContent {
public:
	DebugDrawImgui();
	void ImGui_getContent() override;
};


class Engine : public ImGuiContent {
private:
	// singleton
	static std::unique_ptr<Engine> engineInstance;

	DebugDrawImgui debugdrawImgui;

	Input::InputObserver input;
	//InstanceObserver instanceObs;

	Engine(std::unique_ptr<Window>&& initWindow, EngineRunnable&& toRun, const Vec2u& initWindowSize, EngineSettings engineSettings);

public:
	~Engine() = default;

	static void init(std::unique_ptr<Window>&& window, EngineRunnable&& toRun, const Vec2u& initWindowSize = Vec2u(0, 0), EngineSettings engineSettings = EngineSettings{});
	static void shutdown();

	static bool is_init() { return engineInstance && engineInstance->isInit(); }
	static bool start_running() { return is_init() && engineInstance->run(); }

	static int getWindowScale() noexcept { return engineInstance->windowZoom; }
	static const Window* getWindow() noexcept { return engineInstance->window.get(); }
	static const FixedEngineClock* getClock() { return &engineInstance->clock; }
	static secs getUpTime() { return engineInstance->upTime; }

private:

	void addRunnable(EngineRunnable&& toRun);

	bool run();

	bool isRunning() const noexcept { return running; };
	bool isInit() const noexcept { return initialized; }

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

	float gamespeed = 1.f;

	unsigned int avgFPS = 0;
	unsigned int avgUPS = 0;

	// display management
	std::unique_ptr<Window> window;
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
	bool stepUpdate = false;
	bool pauseInterpolation = false;

	std::vector<EngineRunnable> runnables;

	// run() subcalls
	void initRenderTarget(bool fullscreen);
	void runUpdate(std::barrier<>* bar);
	void drawRunnable(EngineRunnable& run);

	void close();

	void handleEvents(bool* timeWasted);
	void resizeWindow(Vec2u size);
	bool setFullscreen(bool fullscreen);

	void ImGui_getContent();
	void ImGui_getExtraContent();
};

}
