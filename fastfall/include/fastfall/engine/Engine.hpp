#pragma once

#include "time/EngineClock.hpp"
#include "time/FixedEngineClock.hpp"
#include "state/EngineStateHandler.hpp"
#include "EngineRunnable.hpp"

//#include "resource/Resources.hpp"

#include "imgui/ImGuiContent.hpp"

//#include "input/Input.hpp"

#include "fastfall/game/InstanceObserver.hpp"

#include "fastfall/util/Vec2.hpp"

#include "fastfall/render/VertexArray.hpp"
#include "fastfall/render/Window.hpp"
#include "fastfall/engine/input.hpp"

#include <queue>
#include <memory>
#include <mutex>
#include <functional>
#include <chrono>

//#include <SFML/Graphics.hpp>

#include <mutex>
#include <barrier>


//using namespace std::chrono;

namespace ff {

//constexpr int VERSION[3] = { PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH };
constexpr int VERSION[3] = { 0, 0, 1 };

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
	InstanceObserver instanceObs;

	Engine(std::unique_ptr<Window>&& initWindow, EngineRunnable&& toRun, const Vec2u& initWindowSize, EngineSettings engineSettings);

public:

	~Engine() = default;

	static inline Engine& get() { assert(engineInstance != nullptr); return *engineInstance; };

	static void init(std::unique_ptr<Window>&& window, EngineRunnable&& toRun, const Vec2u& initWindowSize = Vec2u(0, 0), EngineSettings engineSettings = EngineSettings{});

	static void shutdown();

	void addRunnable(EngineRunnable&& toRun);

	bool run();

	inline bool isRunning() const noexcept { return running; };
	inline bool isInit() const noexcept { return initialized; }

	void freeze();
	void freezeStepOnce();
	void unfreeze();
	bool isFrozen() const noexcept;

	inline int getWindowScale() const noexcept { return windowZoom; }

	const Window* getWindow() const noexcept { return window.get(); }

private:
	bool run_doubleThread();
	bool run_singleThread();
	bool run_emscripten();
	static void emscripten_loop(void* engine_ptr);

	void prerun_init();

	void updateTimer();
	void updateStateHandler();
	void updateView();
	void updateRunnables();
	void predrawRunnables();
	void drawRunnables();
	void updateImGui();
	void cleanRunnables();
	void sleep();

	//bool showImGui = false;

	EngineSettings settings;

	bool initialized = false;

	// thread-sync helper
	// class Barrier;

	bool running = false;

	float gamespeed = 1.f;

	unsigned int avgFPS = UINT_MAX;

	// display management
	//bool windowless;
	std::unique_ptr<Window> window;
	Vec2i lastWindowedPos;
	Vec2u initWinSize;
	Vec2u lastWindowedSize;
	int windowZoom;

	bool wantFullscreen = false;
	bool hasFocus = true;

	// margins
	std::unique_ptr<VertexArray> margins;
	View marginView;

	// framerate & time management
	//EngineClock clock;
	FixedEngineClock clock;
	unsigned update_counter = 0;
	secs elapsedTime;
	float interpolation;
	bool hasUpdated = false;
	//secs maxDeltaTime;
	//secs deltaTime;

	// event handling
	unsigned event_count = 0u;

	std::chrono::time_point<std::chrono::steady_clock> displayStart;
	std::chrono::duration<float> displayTime;

	bool pauseUpdate, stepUpdate;

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
