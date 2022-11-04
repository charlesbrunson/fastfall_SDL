
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
