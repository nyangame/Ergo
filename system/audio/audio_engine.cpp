#include "audio_engine.hpp"

// miniaudio integration:
// To enable, place miniaudio.h in engine/third_party/ and define ERGO_HAS_MINIAUDIO=1
#ifdef ERGO_HAS_MINIAUDIO
#define MINIAUDIO_IMPLEMENTATION
#include "engine/third_party/miniaudio.h"

struct AudioEngine::Impl {
    ma_engine engine;
    bool engine_initialized = false;
};
#else
struct AudioEngine::Impl {
    // Stub impl when miniaudio is not available
};
#endif

bool AudioEngine::initialize() {
    if (initialized_) return true;
    impl_ = new Impl{};

#ifdef ERGO_HAS_MINIAUDIO
    ma_engine_config config = ma_engine_config_init();
    if (ma_engine_init(&config, &impl_->engine) != MA_SUCCESS) {
        delete impl_;
        impl_ = nullptr;
        return false;
    }
    impl_->engine_initialized = true;
#endif

    initialized_ = true;
    return true;
}

void AudioEngine::shutdown() {
    if (!impl_) return;

#ifdef ERGO_HAS_MINIAUDIO
    if (impl_->engine_initialized) {
        ma_engine_uninit(&impl_->engine);
    }
#endif

    delete impl_;
    impl_ = nullptr;
    initialized_ = false;
}

SoundHandle AudioEngine::load_music(std::string_view /*path*/) {
    // Would load a streaming sound with miniaudio
    return {0};
}

void AudioEngine::play_music(SoundHandle /*handle*/, bool /*loop*/) {}
void AudioEngine::stop_music() {}
void AudioEngine::pause_music() {}
void AudioEngine::resume_music() {}
void AudioEngine::set_music_volume(float /*volume*/) {}

SoundHandle AudioEngine::load_sound(std::string_view /*path*/) {
    return {0};
}

void AudioEngine::play_sound(SoundHandle /*handle*/, float /*volume*/, float /*pitch*/) {}

void AudioEngine::set_master_volume(float volume) {
    master_volume_ = volume;
#ifdef ERGO_HAS_MINIAUDIO
    if (impl_ && impl_->engine_initialized) {
        ma_engine_set_volume(&impl_->engine, volume);
    }
#endif
}

void AudioEngine::update() {
    // miniaudio handles its own threading; this is for future use
}
