
#include "fastfall/engine/Engine.hpp"
#include "fastfall/render/target/Window.hpp"
#include "fastfall/fastfall.hpp"

#include "content/types.hpp"
#include "content/TestState.hpp"


int main(int argc, char* argv[]) {
    register_types();

    // initialize subsystems
    if (ff::Init())
    {
        // create an opengl context
        auto* window = new ff::Window{ true };
        ff::Engine* engine = nullptr;

        // load resources from file
        if (ff::Load_Resources( "data/" ))
        {
            // create the engine
            engine = new ff::Engine{ window };

            // give it something to run
            engine->make_runnable<TestState>();

            // run it
            engine->run();
        }
#if not defined(__EMSCRIPTEN__)


        if (engine)
            delete engine;

        ff::Quit();

        delete window;

#endif
    }

	return EXIT_SUCCESS;
}
