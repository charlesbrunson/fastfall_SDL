
#include "fastfall/util/log.hpp"
#include "fastfall/util/math.hpp"

#include "fastfall/resource/Resources.hpp"
#include "fastfall/resource/ResourceWatcher.hpp"

#include "fastfall/render.hpp"

#include "fastfall/engine/Engine.hpp"
#include "fastfall/engine/imgui/ImGuiFrame.hpp"

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
	settings.runstyle = ff::EngineRunStyle::DoubleThread;
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

	FFinit();

	// need to create window before loading resources :(
	std::unique_ptr<Window> window = std::make_unique<Window>();

	Resources::loadAll(Resources::AssetSource::INDEX_FILE, "fileindex.xml");

	Engine::init(
		std::move(window),
		EngineRunnable(std::make_unique<TestState>()),
		Vec2u(1600, 900),
		getSettings()
	);

	if (Engine::getInstance().isInit()) {

		Engine::getInstance().run();

	}
	else {
		std::cout << "Could not initialize engine" << std::endl;
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
