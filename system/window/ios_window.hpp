#pragma once
#include <cstdint>
#include <string_view>

// iOS window (UIWindow + CAMetalLayer)
class IOSWindow {
    struct Impl;
    Impl* impl_ = nullptr;

public:
    IOSWindow() = default;
    ~IOSWindow();

    bool create(uint32_t width, uint32_t height, std::string_view title);
    bool should_close() const;
    void poll_events();
    uint32_t width() const;
    uint32_t height() const;
    void* get_surface_handle() const;
};
