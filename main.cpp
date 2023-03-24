
#include "fastfall/engine/Engine.hpp"
#include "fastfall/render/target/Window.hpp"
#include "fastfall/fastfall.hpp"

#include "content/types.hpp"
#include "content/TestState.hpp"

int main(int argc, char* argv[]) {
    game_InitTypes();

    // initialize subsystems
    if (ff::Init())
    {
        // create an opengl context
        auto window = ff::Window{ false };

        // load resources from file
        if (ff::Load_Resources( "data/" ))
        {
            // create the engine
            auto engine = ff::Engine{ &window };

            // give it something to run
            engine.make_runnable<TestState>();

            // run it
            engine.run();
        }
        ff::Quit();
    }

	return EXIT_SUCCESS;
}
