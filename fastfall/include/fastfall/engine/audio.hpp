#pragma once

#include "soloud.h"

namespace ff {
    bool audio_init();
    bool audio_quit();
    bool audio_is_init();
    SoLoud::Soloud& audio();
}