
#include <thread>
#include <iostream>

#include "fastfall/render/opengl.hpp"
#include "fastfall/render.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/util/math.hpp"
#include "fastfall/engine/Engine.hpp"
#include "EmptyState.hpp"
#include "content/TestState.hpp"

#include "fastfall/resource/Resources.hpp"
#include "fastfall/resource/ResourceWatcher.hpp"

#include "fastfall/engine/imgui/ImGuiFrame.hpp"

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

	std::unique_ptr<Window> window = std::make_unique<Window>();

	Resources::loadAll(Resources::AssetSource::INDEX_FILE, "fileindex.xml");
	//Resources::buildPackFile("data.pack");
	//Resources::unloadAll();

	//Resources::loadAll(Resources::AssetSource::PACK_FILE, "data.pack");

	//Resources::addLoadedToWatcher();
	//ResourceWatcher::start_watch_thread();

	Engine::init(
		std::move(window),
		EngineRunnable(std::make_unique<TestState>()),
		Vec2u(1920, 1080),
		EngineSettings{
			.allowMargins = true,
			.refreshRate = 144,
			.vsyncEnabled = true,
#if defined(DEBUG)
			.showDebug = true
#else
			.showDebug = false
#endif
		}
	);

	if (Engine::getInstance().isInitialized()) {
		//Engine::getInstance().run_singleThread();
		Engine::getInstance().run_doubleThread();
	}
	else {
		std::cout << "Could not initialize engine" << std::endl;
		return EXIT_FAILURE;
	}

	//ResourceWatcher::stop_watch_thread();
	//ResourceWatcher::clear_watch();
	//ResourceWatcher::join_watch_thread();

	Engine::shutdown();
	Resources::unloadAll();

	ImGuiFrame::getInstance().clear();

	FFquit();

	return EXIT_SUCCESS;
}
