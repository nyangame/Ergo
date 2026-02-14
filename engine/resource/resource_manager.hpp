#pragma once
#include "texture_handle.hpp"
#include "font.hpp"
#include <string>
#include <unordered_map>

class ResourceManager {
public:
    // Texture management (prevents duplicate loads via ref counting)
    TextureHandle load_texture(std::string_view path);
    void release_texture(TextureHandle handle);

    // Font management
    FontAtlas* load_font(std::string_view ttf_path, float size);
    void release_font(std::string_view key);

    // Release resources with ref_count == 0
    void collect_garbage();

    // Release all resources
    void shutdown();

    // Stats
    size_t texture_count() const { return textures_.size(); }
    size_t font_count() const { return fonts_.size(); }

private:
    struct TextureEntry {
        TextureHandle handle;
        uint32_t ref_count = 0;
        std::string path;
    };

    struct FontEntry {
        FontAtlas atlas;
        uint32_t ref_count = 0;
    };

    std::unordered_map<std::string, TextureEntry> textures_;
    std::unordered_map<std::string, FontEntry> fonts_;
    uint64_t next_texture_id_ = 1;
};

inline ResourceManager g_resources;
