

#include "fastfall/util/log.hpp"

#include "fastfall/engine/audio.hpp"
#include "fastfall/resource/Resources.hpp"
#include "fastfall/resource/ResourceWatcher.hpp"

#include "fastfall/engine/Engine.hpp"
#include "fastfall/engine/imgui/ImGuiFrame.hpp"

#include "content/types.hpp"
#include "content/TestState.hpp"
#include "soloud_speech.h"

#ifdef WIN32
#include <Windows.h>

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	_declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
}

#endif

ff::EngineSettings getSettings() {
	ff::EngineSettings settings;
	settings.allowMargins = true;
	settings.fullscreen = false;

#if defined(__EMSCRIPTEN__)
	settings.refreshRate = 0;
	settings.vsyncEnabled = false;
	settings.runstyle = ff::EngineRunStyle::Emscripten;
#else
	settings.refreshRate = 0;
	settings.vsyncEnabled = true;
	settings.runstyle = ff::EngineRunStyle::DoubleThread;
#endif
	settings.showDebug = false;
	return settings;
}

int main(int argc, char* argv[])
{
	srand(time(NULL));

	using namespace ff;

	game_InitTypes();

	render_init();
    audio_init();

	// need to create window before loading resources :(
    Window window;
	if (!window.valid()) {
		LOG_ERR_("Could not initialize window");
		return EXIT_FAILURE;
	}

	bool result = Resources::loadAll(Resources::AssetSource::INDEX_FILE, "fileindex.xml");
	if (!result) {
		LOG_ERR_("Could not load assets");
		return EXIT_FAILURE;
	}

#if not defined(__EMSCRIPTEN__)
	Resources::addLoadedToWatcher();
	ResourceWatcher::start_watch_thread();
#endif

    {
        Engine engine{ &window };
        engine.make_runnable<TestState>();
        engine.run();
    }

#if not defined(__EMSCRIPTEN__)
	ResourceWatcher::stop_watch_thread();

	Resources::unloadAll();
	ImGuiFrame::getInstance().clear();

    audio_quit();
	render_quit();
	ResourceWatcher::join_watch_thread();
#endif

	return EXIT_SUCCESS;
}
