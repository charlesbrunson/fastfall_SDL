
#include "fastfall/util/log.hpp"
#include "fastfall/util/math.hpp"

#include "fastfall/resource/Resources.hpp"
#include "fastfall/resource/ResourceWatcher.hpp"

#include "fastfall/render.hpp"

#include "fastfall/engine/Engine.hpp"
#include "fastfall/engine/imgui/ImGuiFrame.hpp"

#include "content/types.hpp"
#include "content/TestState.hpp"


ff::EngineSettings getSettings() {
	ff::EngineSettings settings;
	settings.allowMargins = true;
	settings.fullscreen = false;
	settings.noWindow = false;
	settings.refreshRate = 144;

#if defined(__EMSCRIPTEN__)
	settings.runstyle = ff::EngineRunStyle::Emscripten;
#else
	settings.runstyle = ff::EngineRunStyle::SingleThread;
	//settings.runstyle = ff::EngineRunStyle::DoubleThread;
#endif

#if defined(DEBUG)
	settings.showDebug = false;
#else
	settings.showDebug = false;
#endif

	settings.vsyncEnabled = true;

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
	if (!result) {
		LOG_ERR_("Could not load assets");
		return EXIT_FAILURE;
	}

	Engine::init(
		std::move(window),
		EngineRunnable(std::make_unique<TestState>()),
		Vec2u(1920, 1080),
		getSettings()
	);

	if (Engine::getInstance().isInit()) {
		Engine::getInstance().run();
	}
	else {
		LOG_ERR_("Could not initialize engine");
		return EXIT_FAILURE;
	}

#if not defined(__EMSCRIPTEN__)
	Resources::unloadAll();
	Engine::shutdown();

	ImGuiFrame::getInstance().clear();

	FFquit();
#endif

	return EXIT_SUCCESS;
}
