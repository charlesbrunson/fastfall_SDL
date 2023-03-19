#pragma once

//#include <SFML/Graphics.hpp>

#include "fastfall/render/target/RenderTexture.hpp"

#include "state/EngineStateHandler.hpp"


// abstracts the state behavior and rendertarget from the engine itself
// engine can drive multiple EngineRunnables at once

namespace ff {

class EngineRunnable {

public:

	EngineRunnable(std::unique_ptr<EngineState> initState, bool useTexture = false);

	// may return nullptr if EngineRunnable isn't using texture
	RenderTexture* getRTexture() const;

	EngineStateHandler& getStateHandle();

	bool isRunning() const;
    bool usesRTexture() const;

private:
	EngineStateHandler stateHandle;

	bool windowless;

	// may point to engine's window or the internally managed sf::RenderTexture
	std::unique_ptr<RenderTexture> rTexPtr;
};

}