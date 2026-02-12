#pragma once
#include <string_view>
#include <cstdint>

struct SoundHandle {
    uint64_t id = 0;
    bool valid() const { return id != 0; }
};

class AudioEngine {
public:
    bool initialize();
    void shutdown();

    // BGM
    SoundHandle load_music(std::string_view path);
    void play_music(SoundHandle handle, bool loop = true);
    void stop_music();
    void pause_music();
    void resume_music();
    void set_music_volume(float volume);

    // Sound effects
    SoundHandle load_sound(std::string_view path);
    void play_sound(SoundHandle handle, float volume = 1.0f, float pitch = 1.0f);

    // Master
    void set_master_volume(float volume);
    float master_volume() const { return master_volume_; }

    // Update (call each frame for streaming, etc.)
    void update();

    bool is_initialized() const { return initialized_; }

private:
    struct Impl;
    Impl* impl_ = nullptr;
    bool initialized_ = false;
    float master_volume_ = 1.0f;
};
