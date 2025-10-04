#include "fastfall/engine/audio.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/resource/Resources.hpp"

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <magic_enum/magic_enum.hpp>

namespace ff::audio {

class Track {
public:
    explicit Track(uint32_t track_id, MIX_Mixer* mixer) : id{track_id} {
        p_impl = MIX_CreateTrack(mixer);
    }
    ~Track() {
        MIX_DestroyTrack(p_impl);
    }

    MIX_Track* ptr() const { return p_impl; }

    uint32_t id;
    uint32_t generation_counter = 0;

    bool in_use = false;

private:
    MIX_Track* p_impl;
};


constexpr uint32_t tracks_min = 8;
constexpr uint32_t tracks_max = 32;

struct {
    bool init_state = false;

    MIX_Mixer* mixer = nullptr;
    std::vector<Track> tracks;

} state;


bool init() {

    if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        LOG_INFO("Failed to init SDL audio subsystem: {}", SDL_GetError());
        return false;
    }

    if (!MIX_Init())
    {
        LOG_INFO("Failed to init SDL audio mixer: {}", SDL_GetError());
        return false;
    }

    SDL_AudioSpec spec;
    spec.channels = 2;
    spec.freq     = 44100;
    spec.format   = SDL_AUDIO_F32LE;

    if (state.mixer) MIX_DestroyMixer(state.mixer);
    state.mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);


    state.tracks.clear();
    state.tracks.reserve(tracks_min);
    for (uint32_t i = 0; i < tracks_min; i++) {
        state.tracks.emplace_back(i, state.mixer);
    }

    LOG_INFO("Initialized audio");

    state.init_state = true;
    return state.init_state;
}

void quit() {
    state.tracks.clear();

    MIX_DestroyMixer(state.mixer);
    state.mixer = nullptr;

    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    state.init_state = false;
}

bool is_init() { return state.init_state; }

void set_master_volume(float volume) {
    MIX_SetMasterGain(state.mixer, volume);
}

float get_master_volume() {
    return MIX_GetMasterGain(state.mixer);
}

MIX_Mixer* get_mixer() {
    return state.mixer;
}

}

namespace ff {

SoundHandle::SoundHandle(const char* tag, MIX_Group* group) {

    audio::Track* use_track = nullptr;
    for (auto& track : audio::state.tracks) {
        if (!track.in_use || !MIX_TrackPlaying(track.ptr())) {
            use_track = &track;
            break;
        }
    }

    if (!use_track) {
        use_track = &audio::state.tracks.emplace_back(
            audio::state.tracks.size(),
            audio::state.mixer);
    }

    if (use_track) {
        use_track->in_use = true;
        use_track->generation_counter++;

        track_id   = use_track->id;
        generation = use_track->generation_counter;
    }
}

SoundHandle::~SoundHandle() {
    if (generation > 0) {
        audio::state.tracks[track_id].in_use = false;
    }
}

bool SoundHandle::set_sound(const SoundAsset& asset) {
    if (generation > 0) {
        auto& track = audio::state.tracks[track_id];
        return MIX_SetTrackAudio(track.ptr(), asset.get_data());
    }
    return false;
}

bool SoundHandle::apply_config() {
    if (generation > 0) {
        auto& track = audio::state.tracks[track_id];

        MIX_SetTrackGain(track.ptr(), config.gain);
    }
    return false;
}


bool SoundHandle::play() {
    if (generation > 0) {
        auto& track = audio::state.tracks[track_id];
        return MIX_PlayTrack(track.ptr(), {});
    }
    return false;
}

bool SoundHandle::stop() {
    if (generation > 0) {
        auto& track = audio::state.tracks[track_id];
        return MIX_StopTrack(track.ptr(), {});
    }
    return false;
}


}
