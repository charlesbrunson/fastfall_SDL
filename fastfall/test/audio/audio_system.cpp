#include "gtest/gtest.h"

#include "fastfall/util/log.hpp"
#include "fastfall/engine/audio.hpp"
#include "fastfall/resource/asset/SoundAsset.hpp"
#include "fastfall/game/systems/AudioSystem.hpp"
#include "soloud_speech.h"

#include <thread>

using namespace ff;


TEST(audio, init)
{
    audio::init(audio::AudioBackends::Null);
    EXPECT_TRUE(audio::is_init());
    audio::quit();
    EXPECT_TRUE(!audio::is_init());
}

TEST(audio, system)
{
    using namespace std::chrono_literals;
    audio::init(audio::AudioBackends::SDL2);

    SoundAsset sound{ "data/Bump.wav" };
    if (!sound.loadFromFile()) {
        LOG_ERR_("Failed to load {}", "data/Bump.wav");
    }

    AudioSystem sys{ &audio::primary_bus() };

    for (int v = 100; v > 0; v -= 10) {
        sys.play(sound,
                 audio::Volume{ (float)v * 0.01f },
                 audio::Pan{ -1.f + ((float)v / 50.f) }
                 );

        LOG_INFO("Playing {} at {}%", sound.get_name(), v);
        std::this_thread::sleep_for(0.5s);
        sys.update(0.5);
    }

    audio::quit();
}
