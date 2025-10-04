#pragma once

#include <unordered_set>
#include "fastfall/util/log.hpp"

// #include "miniaudio.h"
#include "fastfall/engine/audio.hpp"
#include "fastfall/engine/time/time.hpp"

namespace ff {

    class AudioSystem {
    public:

        std::optional<SoundHandle> play(std::string_view sound_asset_name, SoundCfg cfg = {}) {
            if (SoundAsset* asset = get_asset_impl(sound_asset_name))
            {
                return play(*asset, cfg);
            }
            return std::nullopt;
        }

        std::optional<SoundHandle> play(SoundAsset& sound_asset, SoundCfg cfg = {}) {
            SoundHandle sound_handle{"game"};
            sound_handle.config = cfg;
            sound_handle.apply_config();
            sound_handle.set_sound(sound_asset);
            sound_handle.play();
            return sound_handle;
        }

        void update(secs deltaTime);

    private:
        SoundAsset* get_asset_impl(std::string_view sound_asset_name);

        secs upTime = 0.0;
    };

}

