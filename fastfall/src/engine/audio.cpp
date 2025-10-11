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

    void set_tag(const char* ntag) {
        if (tag) { MIX_UntagTrack(ptr(), tag); }
        tag = ntag;
        if (tag) { MIX_TagTrack(ptr(), tag); }
    }

    void clear_tag() {
        if (tag) { MIX_UntagTrack(ptr(), tag); }
        tag = nullptr;
    }

    uint32_t id;
    uint32_t generation_counter = 0;

    bool in_use = false;

    const SoundAsset* source = nullptr;

private:
    const char* tag = nullptr;
    MIX_Track* p_impl;
};


constexpr uint32_t tracks_min = 16;
constexpr uint32_t tracks_max = 16;

struct {
    bool init_state = false;

    bool muted = false;
    float gain = 1.0f;

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
    state.gain = volume;
    if (!state.muted) {
        MIX_SetMasterGain(state.mixer, volume);
    }
}

float get_master_volume() {
    return state.gain;
}

MIX_Mixer* get_mixer() {
    return state.mixer;
}

}

namespace ff {

SoundHandle::SoundHandle(const char* tag, MIX_Group* group) {

    audio::Track* use_track = nullptr;
    for (auto& track : audio::state.tracks) {
        if (!track.in_use && !MIX_TrackPlaying(track.ptr())) {
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

SoundHandle::SoundHandle(SoundHandle&& other) noexcept {
    track_id = other.track_id;
    generation = other.generation;
    tag = other.tag;
    group = other.group;

    other.track_id = 0;
    other.generation = 0;
    other.tag = nullptr;
    other.group = nullptr;
}

SoundHandle& SoundHandle::operator=(SoundHandle&& other) noexcept {
    track_id = other.track_id;
    generation = other.generation;
    tag = other.tag;
    group = other.group;

    other.track_id = 0;
    other.generation = 0;
    other.tag = nullptr;
    other.group = nullptr;

    return *this;
}

SoundHandle::~SoundHandle() {
    if (generation > 0) {
        audio::state.tracks[track_id].in_use = false;
    }
}

bool SoundHandle::set_sound(const SoundAsset& asset) {
    if (generation > 0) {
        auto& track = audio::state.tracks[track_id];
        track.source = &asset;
        return MIX_SetTrackAudio(track.ptr(), asset.get_data());
    }
    return false;
}

bool SoundHandle::apply_config() {
    if (generation > 0) {
        auto& track = audio::state.tracks[track_id];

        MIX_SetTrackGain(track.ptr(), config.gain);
        track.set_tag(config.tag);
        MIX_SetTrackGroup(track.ptr(), config.group);
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

AudioImGui::AudioImGui() :
    ImGuiContent(ImGuiContentType::SIDEBAR_LEFT, "Audio", "System")
{
}

void AudioImGui::ImGui_getContent(secs deltaTime) {

    using namespace audio;

    if (ImGui::Checkbox("Mute", &state.muted)) {
        MIX_SetMasterGain(state.mixer, state.muted ? 0.f : state.gain);
    }

    float master_gain = get_master_volume();
    if (ImGui::SliderFloat("Master Gain", &master_gain, 0.0f, 2.0f)) {
        set_master_volume(master_gain);
    }

    ImGui::Text("Tracks (%lu)", state.tracks.size());
    for (auto& track : state.tracks) {

        MIX_Track* trackptr = track.ptr();
        ImGui::PushID(trackptr);

        bool in_use = track.in_use || MIX_TrackPlaying(trackptr);
        ImGui::Checkbox("", &in_use);

        ImGui::SameLine();
        int64_t pos_ms = 0;
        int64_t len_ms = 1;
        int64_t zero = 0;

        if (in_use) {
            pos_ms = MIX_TrackFramesToMS(trackptr, MIX_GetTrackPlaybackPosition(trackptr));
            if (pos_ms != -1) {
                len_ms = pos_ms + MIX_TrackFramesToMS(trackptr, MIX_GetTrackRemaining(trackptr));
            }
            else {
                pos_ms = 0;
            }
        }

        // static char track_name[128] = {};
        // memset(track_name, 0, sizeof(track_name));

        const SoundAsset* sound_asset = track.source && in_use ? track.source : nullptr;

        // fmt::format_to_n(track_name, 128, "{}", sound_asset ? sound_asset->get_name() : "");
        // ImGui::SliderScalar(track_name, ImGuiDataType_S64, &pos_ms, &zero, &len_ms, "%dms");

        static char overlay[128] = {};
        memset(overlay, 0, sizeof(overlay));
        fmt::format_to_n(overlay, 128, "{}ms", pos_ms);

        ImGui::ProgressBar(
            (float)pos_ms / (float)len_ms,
            ImVec2(0.f, ImGui::GetFrameHeight()),
            overlay);

        ImGui::SameLine();
        ImGui::Text("%s", sound_asset ? sound_asset->get_name().data() : "");

        ImGui::PopID();
    }
}


}
