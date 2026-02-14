#include "image_loader.hpp"
#include <fstream>
#include <cstring>

// stb_image integration:
// To enable, place stb_image.h in engine/third_party/ and define ERGO_HAS_STB_IMAGE=1
#ifdef ERGO_HAS_STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"
#endif

ImageData load_image(std::string_view path) {
#ifdef ERGO_HAS_STB_IMAGE
    int w, h, channels;
    unsigned char* data = stbi_load(std::string(path).c_str(), &w, &h, &channels, 4);
    if (!data) return {};
    ImageData img;
    img.width = static_cast<uint32_t>(w);
    img.height = static_cast<uint32_t>(h);
    img.channels = 4;
    img.pixels.assign(data, data + w * h * 4);
    stbi_image_free(data);
    return img;
#else
    (void)path;
    return {};
#endif
}

ImageData load_image_from_memory(const uint8_t* data, size_t size) {
#ifdef ERGO_HAS_STB_IMAGE
    int w, h, channels;
    unsigned char* pixels = stbi_load_from_memory(data, static_cast<int>(size),
                                                   &w, &h, &channels, 4);
    if (!pixels) return {};
    ImageData img;
    img.width = static_cast<uint32_t>(w);
    img.height = static_cast<uint32_t>(h);
    img.channels = 4;
    img.pixels.assign(pixels, pixels + w * h * 4);
    stbi_image_free(pixels);
    return img;
#else
    (void)data; (void)size;
    return {};
#endif
}
