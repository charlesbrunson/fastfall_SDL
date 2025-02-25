#pragma once

#include <unordered_set>
#include <memory>
#include <fastfall/util/log.hpp>

#include "miniaudio.h"
#include "fastfall/engine/time/time.hpp"
#include "fastfall/resource/asset/SoundAsset.hpp"

namespace ff {

    struct SoundHandle {
        ma_sound* sound;
        ma_resource_manager_data_source* data_source;

        bool operator==(const SoundHandle& other) const {
            return sound == other.sound;
        }
    };

    struct SoundHandleHash
    {
        std::size_t operator()(const SoundHandle& s) const noexcept
        {
            return std::hash<void*>{}(s.sound);
        }
    };

    namespace audio {
        struct Volume {
            float loudness = 1.f;
            void operator()(ma_sound* t_hdl);
        };
        struct Pan {
            float pan = 1.f;
            void operator()(ma_sound* t_hdl);
        };

        struct game_bus_t;
    }

    template<typename T>
    concept is_audio_property = requires (T x, ma_sound* hdl) {
        x(hdl);
    };


    class AudioSystem {
    public:

        template<is_audio_property ...AudioProp_Ts>
        std::optional<SoundHandle> play(std::string_view sound_asset_name, AudioProp_Ts&&... props) {
            if (SoundAsset* asset = get_asset_impl(sound_asset_name))
            {
                return play(*asset, std::forward<AudioProp_Ts>(props)...);
            }
            return std::nullopt;
        }

        template<is_audio_property ...AudioProp_Ts>
        std::optional<SoundHandle> play(SoundAsset& sound_asset, AudioProp_Ts&&... props) {
            auto hdl = play_impl(sound_asset);
            if (hdl.sound && hdl.data_source) {
                set(hdl.sound, std::forward<AudioProp_Ts>(props)...);
                ma_sound_start(hdl.sound);
                auto [it, inserted] = active_sounds.emplace(hdl);
                return inserted ? std::make_optional(*it) : std::nullopt;
            }
            else {
                return std::nullopt;
            }
        }

        inline void pause_all()   { set_pause_all(true);  };
        inline void unpause_all() { set_pause_all(false); };
        void set_pause_all(bool t_paused);

        inline void pause(ma_sound* t_hdl)   { set_pause(t_hdl, true);  };
        inline void unpause(ma_sound* t_hdl) { set_pause(t_hdl,false); };
        void set_pause(ma_sound* t_hdl, bool t_paused);

        void stop(ma_sound* t_hdl);

        template<is_audio_property ...AudioProp_Ts>
        void set(ma_sound* t_hdl, AudioProp_Ts&&... props) {
            (props(t_hdl), ...);
        }

        void update(secs deltaTime);

        bool valid_sound(SoundHandle hdl) const { return active_sounds.contains(hdl); }

    private:
        std::unordered_set<SoundHandle, SoundHandleHash> active_sounds;

        SoundHandle play_impl(SoundAsset& sound_asset);
        SoundAsset* get_asset_impl(std::string_view sound_asset_name);

        secs upTime = 0.0;
    };

}

