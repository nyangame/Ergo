#pragma once
#include <cstdint>
#include <string_view>

// Android window (ANativeWindow)
class AndroidWindow {
    struct Impl;
    Impl* impl_ = nullptr;

public:
    AndroidWindow() = default;
    ~AndroidWindow();

    bool create(uint32_t width, uint32_t height, std::string_view title);
    bool should_close() const;
    void poll_events();
    uint32_t width() const;
    uint32_t height() const;
    void* get_surface_handle() const;
};
