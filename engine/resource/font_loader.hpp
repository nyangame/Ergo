#pragma once
#include "font.hpp"
#include <vector>
#include <string_view>
#include <utility>

class FontLoader {
public:
    // Load a TTF file and generate a font atlas bitmap.
    // char_ranges: list of (first, last) codepoint pairs to include.
    // Returns atlas with glyph data populated. The atlas bitmap pixels are
    // stored in the returned ImageAtlasData; the caller must upload to GPU
    // and set atlas.texture accordingly.
    struct ImageAtlasData {
        uint32_t width = 0;
        uint32_t height = 0;
        std::vector<uint8_t> pixels;  // single-channel (alpha)
    };

    struct LoadResult {
        FontAtlas atlas;
        ImageAtlasData image;
        bool valid = false;
    };

    static LoadResult load(
        std::string_view ttf_path,
        float font_size,
        const std::vector<std::pair<uint32_t, uint32_t>>& char_ranges);

    // Default ASCII + Japanese character ranges
    static std::vector<std::pair<uint32_t, uint32_t>> default_ranges() {
        return {
            {0x0020, 0x007E},  // ASCII
            {0x3000, 0x303F},  // CJK symbols
            {0x3040, 0x309F},  // Hiragana
            {0x30A0, 0x30FF},  // Katakana
        };
    }
};
