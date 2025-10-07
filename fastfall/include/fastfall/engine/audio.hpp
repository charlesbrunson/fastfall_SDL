#pragma once

#include "fastfall/resource/asset/SoundAsset.hpp"

struct MIX_Mixer;
struct MIX_Group;

namespace ff {
struct SoundCfg {
    float gain = 1.f;
    const char* tag = nullptr;
    MIX_Group* group = nullptr;
};

class SoundHandle {
public:
    explicit SoundHandle(const char* tag = nullptr, MIX_Group* group = nullptr);
    SoundHandle(const SoundHandle&) = delete;
    SoundHandle& operator=(const SoundHandle&) = delete;
    SoundHandle(SoundHandle&& other) noexcept;
    SoundHandle& operator=(SoundHandle&& other) noexcept;
    ~SoundHandle();

    SoundCfg config;

    bool apply_config();

    bool set_sound(const SoundAsset& asset);
    bool play();
    bool stop();

    size_t hash() const {
        return (static_cast<size_t>(track_id) << 32) | static_cast<size_t>(generation);
    }

    bool operator==(const SoundHandle& other) const {
        return track_id == other.track_id && generation == other.generation;
    }

private:

    uint32_t track_id = 0;
    uint32_t generation = 0;

    const char* tag = nullptr;
    MIX_Group* group = nullptr;
};

}

namespace std {

template<>
struct hash<ff::SoundHandle> {
    size_t operator()(const ff::SoundHandle& hdl) const {
        return hdl.hash();
    }
};

}

namespace ff::audio {


bool init();
void quit();
bool is_init();

void set_master_volume(float volume);
void set_game_volume(float volume);
void set_music_volume(float volume);

float get_master_volume();
float get_game_volume();
float get_music_volume();

MIX_Mixer* get_mixer();

}