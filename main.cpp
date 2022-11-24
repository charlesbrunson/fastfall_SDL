
#include "fastfall/engine/Engine.hpp"
#include "fastfall/render/Window.hpp"
#include "fastfall/fastfall.hpp"

#include "content/types.hpp"
#include "content/TestState.hpp"

int main(int argc, char* argv[])
{
    game_InitTypes();

    // initialize subsystems
    ff::Init();

    // create an opengl context
    auto window = new ff::Window();

    // load resources from file
    ff::Load_Resources();

    // create the engine
    auto engine = new ff::Engine( window );

    // give it something to run
    engine->make_runnable<TestState>();

    // run it
    engine->run();

    // clean up
#if !defined(__EMSCRIPTEN__)
    ff::Quit();
    delete engine;
    delete window;
#endif

	return EXIT_SUCCESS;
}
