#pragma once
#include <cstdint>
#include <string_view>

namespace utf8 {

// Decode one UTF-8 codepoint from the given byte sequence.
// Advances 'pos' past the decoded codepoint.
// Returns 0xFFFD (replacement character) on invalid input.
inline uint32_t decode(std::string_view str, size_t& pos) {
    if (pos >= str.size()) return 0;

    uint8_t c = static_cast<uint8_t>(str[pos]);

    // 1-byte (ASCII)
    if (c < 0x80) {
        ++pos;
        return c;
    }

    uint32_t codepoint = 0;
    int extra_bytes = 0;

    if ((c & 0xE0) == 0xC0) {
        codepoint = c & 0x1F;
        extra_bytes = 1;
    } else if ((c & 0xF0) == 0xE0) {
        codepoint = c & 0x0F;
        extra_bytes = 2;
    } else if ((c & 0xF8) == 0xF0) {
        codepoint = c & 0x07;
        extra_bytes = 3;
    } else {
        ++pos;
        return 0xFFFD;
    }

    if (pos + extra_bytes >= str.size()) {
        pos = str.size();
        return 0xFFFD;
    }

    for (int i = 0; i < extra_bytes; ++i) {
        ++pos;
        uint8_t b = static_cast<uint8_t>(str[pos]);
        if ((b & 0xC0) != 0x80) return 0xFFFD;
        codepoint = (codepoint << 6) | (b & 0x3F);
    }
    ++pos;

    return codepoint;
}

// Count the number of codepoints in a UTF-8 string.
inline size_t count_codepoints(std::string_view str) {
    size_t count = 0;
    size_t pos = 0;
    while (pos < str.size()) {
        decode(str, pos);
        ++count;
    }
    return count;
}

} // namespace utf8
