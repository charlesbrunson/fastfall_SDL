#pragma once

#include "miniaudio.h"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/resource/asset/SoundAsset.hpp"

namespace ff {

    struct SoundHandle {
        unsigned id = 0;
        ma_sound sound;
    };

    namespace audio {
        struct Volume {
            float loudness = 1.f;
            void operator()(SoundHandle t_hdl);
        };
        struct Pan {
            float pan = 1.f;
            void operator()(SoundHandle t_hdl);
        };

        struct game_bus_t;
    }

    template<typename T>
    concept is_audio_property = requires (T x, SoundHandle hdl) {
        x(hdl);
    };


    class AudioSystem {
    public:

        template<is_audio_property ...AudioProp_Ts>
        SoundHandle play(std::string_view sound_asset_name, AudioProp_Ts&&... props) {
            if (SoundAsset* asset = get_asset_impl(sound_asset_name))
            {
                return play(*asset, std::forward<AudioProp_Ts>(props)...);
            }
            return { 0 };
        }

        template<is_audio_property ...AudioProp_Ts>
        SoundHandle play(SoundAsset& sound_asset, AudioProp_Ts&&... props) {
            SoundHandle hdl = play_impl(sound_asset);
            if (hdl.id) {
                set(hdl, std::forward<AudioProp_Ts>(props)...);
                active_sounds.push_back(hdl);
            }
            return hdl;
        }

        inline void pause_all()   { set_pause_all(true);  };
        inline void unpause_all() { set_pause_all(false); };
        void set_pause_all(bool t_paused);

        inline void pause(SoundHandle t_hdl)   { set_pause(t_hdl, true);  };
        inline void unpause(SoundHandle t_hdl) { set_pause(t_hdl,false); };
        void set_pause(SoundHandle t_hdl, bool t_paused);

        void stop(SoundHandle t_hdl);

        template<is_audio_property ...AudioProp_Ts>
        void set(SoundHandle t_hdl, AudioProp_Ts&&... props) {
            (props(t_hdl), ...);
        }

        void update(secs deltaTime);

    private:
        std::vector<SoundHandle> active_sounds; // currently playing ( as of last update() )

        SoundHandle play_impl(SoundAsset& sound_asset);
        SoundAsset* get_asset_impl(std::string_view sound_asset_name);

        secs upTime = 0.0;

        unsigned id_counter = 0;
    };

}