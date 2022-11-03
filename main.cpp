
#include "fastfall/engine/Engine.hpp"
#include "fastfall/render/Window.hpp"
#include "fastfall/fastfall.hpp"

#include "content/types.hpp"
#include "content/TestState.hpp"

/*
#ifdef WIN32
#include <Windows.h>

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	_declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
}
#endif
*/

/*
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
*/

int main(int argc, char* argv[])
{
    game_InitTypes();

    // initialize subsystems
    ff::Init();

    // create an opengl context
    ff::Window window;

    // load resources from file
    ff::Load_Resources();

    // create the engine
    ff::Engine engine{ &window };

    // give it something to run
    engine.make_runnable<TestState>();

    // run it
    engine.run();

    // clean up
    ff::Quit();

	return EXIT_SUCCESS;
}
