#pragma once

#include "fastfall/engine/audio.hpp"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/resource/asset/SoundAsset.hpp"

namespace ff {

    struct SoundHandle {
        unsigned int id;
    };

    namespace audio {
        struct Volume {
            float loudness = 1.f;

            void operator()(SoundHandle t_hdl) {
                engine().setVolume(t_hdl.id, loudness);
            };
        };
    }

    template<typename T>
    concept is_audio_property = requires (T x, SoundHandle hdl) {
        x(hdl);
    };

    class AudioSystem {
    public:

        AudioSystem() = default;

        template<is_audio_property ...AudioProp_Ts>
        SoundHandle play(std::string_view sound_asset_name, AudioProp_Ts&&... props) {
            if (auto* sound = Resources::get<SoundAsset>(sound_asset_name))
            {
                return play(*sound, std::forward<AudioProp_Ts>(props)...);
            }
            return { 0 };
        }

        template<is_audio_property ...AudioProp_Ts>
        SoundHandle play(SoundAsset& sound_asset, AudioProp_Ts&&... props) {
            SoundHandle hdl = { .id = dest->game.play(sound_asset.wav()) };
            set(hdl, std::forward<AudioProp_Ts>(props)...);
            active_sounds.push_back(hdl);
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

        void update(World& world, secs deltaTime);

    private:
        std::vector<SoundHandle> active_sounds; // currently playing ( as of last update() )

        audio::game_bus_t* dest = &audio::primary_bus();
    };

}