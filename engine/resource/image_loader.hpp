#pragma once
#include <cstdint>
#include <vector>
#include <string_view>

struct ImageData {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t channels = 4;
    std::vector<uint8_t> pixels;

    bool valid() const { return !pixels.empty(); }
};

ImageData load_image(std::string_view path);
ImageData load_image_from_memory(const uint8_t* data, size_t size);
