#pragma once
#include <cstdint>
#include <string_view>

// Replaces CppSampleGame's ChangeWindowMode, DxLib_Init, etc.
class DesktopWindow {
    struct Impl;
    Impl* impl_ = nullptr;

public:
    DesktopWindow() = default;
    ~DesktopWindow();

    // Satisfies WindowBackend concept
    bool create(uint32_t width, uint32_t height, std::string_view title);
    bool should_close() const;
    void poll_events();
    uint32_t width() const;
    uint32_t height() const;

    // Vulkan surface handle
    void* get_surface_handle() const;
};
