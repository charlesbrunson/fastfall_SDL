#pragma once

#include "application_list.hpp"

namespace ff {

class application_runner {
public:
    application_runner(std::unique_ptr<application> init_app);

	// may return nullptr if EngineRunnable isn't using texture
	// RenderTexture* getRTexture() const;

	// EngineStateHandler& getStateHandle();

	// bool isRunning() const;
    inline bool is_running() const { return !app_list.empty(); }
    // bool usesRTexture() const;

private:
    application_list app_list;

	// bool windowless;

	// may point to engine's window or the internally managed sf::RenderTexture
	// std::unique_ptr<RenderTexture> rTexPtr;
};

}