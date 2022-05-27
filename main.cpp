
#include "fastfall/util/log.hpp"

#include "fastfall/resource/Resources.hpp"
#include "fastfall/resource/ResourceWatcher.hpp"

#include "fastfall/engine/Engine.hpp"
#include "fastfall/engine/imgui/ImGuiFrame.hpp"

#include "content/types.hpp"
#include "content/TestState.hpp"

#include "fastfall/render/freetype.hpp"
#include "fastfall/render/Font.hpp"
#include "fastfall/render/Text.hpp"

#include "fastfall/game/level/TileID.hpp"

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
	settings.noWindow = false;

#if defined(__EMSCRIPTEN__)
	settings.refreshRate = 0;
	settings.vsyncEnabled = false;
	settings.runstyle = ff::EngineRunStyle::Emscripten;
#else
	settings.refreshRate = 0;
	settings.vsyncEnabled = true;
	settings.runstyle = ff::EngineRunStyle::SingleThread;
	//settings.runstyle = ff::EngineRunStyle::DoubleThread;
#endif

#if defined(DEBUG)
	settings.showDebug = true;
#else
	settings.showDebug = false;
#endif

	return settings;
}


int main(int argc, char* argv[])
{
	srand(time(NULL));
	{
		log::scope scope;
		LOG_VERB("TEST");
		LOG_INFO("TEST");
		LOG_WARN("TEST");
		LOG_ERR_("TEST");
	}
	log::set_verbosity(log::level::INFO);

	using namespace ff;

	game_InitTypes();

	FFinit();

	// need to create window before loading resources :(
	std::unique_ptr<Window> window = std::make_unique<Window>();
	if (!window->valid()) {
		LOG_ERR_("Could not initialize window");
		return EXIT_FAILURE;
	}

	bool result = Resources::loadAll(Resources::AssetSource::INDEX_FILE, "fileindex.xml");
	//result &= Resources::buildPackFile("data.pack");
	//Resources::unloadAll();
	//result &= Resources::loadAll(Resources::AssetSource::PACK_FILE, "data.pack");
	if (!result) {
		LOG_ERR_("Could not load assets");
		return EXIT_FAILURE;
	}

#if not defined(__EMSCRIPTEN__)
	Resources::addLoadedToWatcher();
	ResourceWatcher::start_watch_thread();
#endif

	Engine::init(
		std::move(window),
		EngineRunnable(std::make_unique<TestState>()),
		Vec2u{ GAME_W * 7, GAME_H * 5 },
		getSettings()
	);

	if (Engine::get().isInit()) {
		Engine::get().run();
	}
	else {
		LOG_ERR_("Could not initialize engine");
		return EXIT_FAILURE;
	}

#if not defined(__EMSCRIPTEN__)
	ResourceWatcher::stop_watch_thread();
	ResourceWatcher::join_watch_thread();

	Resources::unloadAll();
	Engine::shutdown();
	ImGuiFrame::getInstance().clear();

	FFquit();
#endif

	return EXIT_SUCCESS;
}
