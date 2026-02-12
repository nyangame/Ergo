#include "resource_manager.hpp"
#include <algorithm>

TextureHandle ResourceManager::load_texture(std::string_view path) {
    std::string key(path);
    auto it = textures_.find(key);
    if (it != textures_.end()) {
        ++it->second.ref_count;
        return it->second.handle;
    }

    TextureEntry entry;
    entry.handle = {next_texture_id_++};
    entry.ref_count = 1;
    entry.path = key;
    // Actual GPU texture creation would happen here via the renderer
    textures_[key] = entry;
    return entry.handle;
}

void ResourceManager::release_texture(TextureHandle handle) {
    for (auto& [path, entry] : textures_) {
        if (entry.handle.id == handle.id) {
            if (entry.ref_count > 0) {
                --entry.ref_count;
            }
            return;
        }
    }
}

FontAtlas* ResourceManager::load_font(std::string_view ttf_path, float size) {
    std::string key = std::string(ttf_path) + ":" + std::to_string(static_cast<int>(size));
    auto it = fonts_.find(key);
    if (it != fonts_.end()) {
        ++it->second.ref_count;
        return &it->second.atlas;
    }

    FontEntry entry;
    entry.atlas.font_size = size;
    entry.ref_count = 1;
    fonts_[key] = std::move(entry);
    return &fonts_[key].atlas;
}

void ResourceManager::release_font(std::string_view key) {
    auto it = fonts_.find(std::string(key));
    if (it != fonts_.end() && it->second.ref_count > 0) {
        --it->second.ref_count;
    }
}

void ResourceManager::collect_garbage() {
    for (auto it = textures_.begin(); it != textures_.end(); ) {
        if (it->second.ref_count == 0) {
            // Actual GPU texture destruction would happen here
            it = textures_.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = fonts_.begin(); it != fonts_.end(); ) {
        if (it->second.ref_count == 0) {
            it = fonts_.erase(it);
        } else {
            ++it;
        }
    }
}

void ResourceManager::shutdown() {
    textures_.clear();
    fonts_.clear();
}
