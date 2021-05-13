
#include "fastfall/engine/config.hpp"
#include "fastfall/engine/EngineRunnable.hpp"

namespace ff {

EngineRunnable::EngineRunnable(std::unique_ptr<EngineState> initState, bool useTexture) :
	stateHandle{ std::move(initState) },
	windowless(useTexture)
{
	if (useTexture) {
		rTexPtr = std::make_unique<RenderTexture>();
		rTexPtr->create(GAME_W, GAME_H);
	}
}

// may return nullptr if EngineRunnable isn't using texture
RenderTexture* EngineRunnable::getRTexture() const {
	return windowless ? rTexPtr.get() : nullptr;
};

EngineStateHandler& EngineRunnable::getStateHandle() {
	return stateHandle;
};

const bool EngineRunnable::isRunning() const {
	return stateHandle.empty();
};

}