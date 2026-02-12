#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "font_asset.hpp"
#include "text_style.hpp"
#include "../core/id_generator.hpp"

// フォントアセット登録パラメータ
struct FontAssetDesc {
    std::string name;                 // アセット名 (検索用)
    std::string source_path;          // フォントファイルパス (.ttf, .otf)
    float base_size = 32.0f;          // MSDF 基本サイズ
    FontRenderMode render_mode = FontRenderMode::MSDF;
    AtlasPopulationMode population_mode = AtlasPopulationMode::Dynamic;
    uint32_t atlas_width = 512;
    uint32_t atlas_height = 512;
    uint32_t atlas_padding = 4;
    std::string preload_chars;        // 事前読み込み文字列 (Static/Preload用)
};

// フォントレジストリ: フォントアセットとテキストマテリアルの一元管理
// Siv3D の FontManager / TextMeshPro の TMP_FontAssetManager に相当
//
// エンジン全体でフォントアセットを共有するためのシングルトンレジストリ。
// スレッドセーフな登録・検索を提供。
class FontRegistry {
    std::unordered_map<uint64_t, FontAsset> fonts_;
    std::unordered_map<std::string, uint64_t> name_to_id_;
    std::unordered_map<uint64_t, TextMaterial> materials_;
    std::unordered_map<std::string, uint64_t> material_name_to_id_;
    mutable std::mutex mutex_;

public:
    FontRegistry() = default;

    // フォントアセットの登録
    // 戻り値: 新規フォントアセットのハンドル
    FontHandle register_font(const FontAssetDesc& desc) {
        std::lock_guard lock(mutex_);

        uint64_t id = IdGenerator::next();
        FontAsset asset;
        asset.id = id;
        asset.name = desc.name;
        asset.source_path = desc.source_path;
        asset.base_size = desc.base_size;
        asset.atlas.render_mode = desc.render_mode;
        asset.atlas.population_mode = desc.population_mode;
        asset.atlas.atlas_width = desc.atlas_width;
        asset.atlas.atlas_height = desc.atlas_height;
        asset.atlas.padding = desc.atlas_padding;

        fonts_[id] = std::move(asset);
        name_to_id_[desc.name] = id;
        return FontHandle{id};
    }

    // フォントアセットの登録解除
    void unregister_font(FontHandle handle) {
        std::lock_guard lock(mutex_);
        auto it = fonts_.find(handle.id);
        if (it != fonts_.end()) {
            name_to_id_.erase(it->second.name);
            fonts_.erase(it);
        }
    }

    // IDで検索
    FontAsset* get_font(FontHandle handle) {
        std::lock_guard lock(mutex_);
        auto it = fonts_.find(handle.id);
        return (it != fonts_.end()) ? &it->second : nullptr;
    }

    const FontAsset* get_font(FontHandle handle) const {
        std::lock_guard lock(mutex_);
        auto it = fonts_.find(handle.id);
        return (it != fonts_.end()) ? &it->second : nullptr;
    }

    // 名前で検索
    FontHandle find_font(std::string_view name) const {
        std::lock_guard lock(mutex_);
        auto it = name_to_id_.find(std::string(name));
        if (it == name_to_id_.end()) return FontHandle{0};
        return FontHandle{it->second};
    }

    // テキストマテリアルの登録
    TextMaterialHandle register_material(const std::string& name, const TextStyle& style) {
        std::lock_guard lock(mutex_);

        uint64_t id = IdGenerator::next();
        TextMaterial mat;
        mat.id = id;
        mat.name = name;
        mat.style = style;

        materials_[id] = std::move(mat);
        material_name_to_id_[name] = id;
        return TextMaterialHandle{id};
    }

    void unregister_material(TextMaterialHandle handle) {
        std::lock_guard lock(mutex_);
        auto it = materials_.find(handle.id);
        if (it != materials_.end()) {
            material_name_to_id_.erase(it->second.name);
            materials_.erase(it);
        }
    }

    TextMaterial* get_material(TextMaterialHandle handle) {
        std::lock_guard lock(mutex_);
        auto it = materials_.find(handle.id);
        return (it != materials_.end()) ? &it->second : nullptr;
    }

    const TextMaterial* get_material(TextMaterialHandle handle) const {
        std::lock_guard lock(mutex_);
        auto it = materials_.find(handle.id);
        return (it != materials_.end()) ? &it->second : nullptr;
    }

    TextMaterialHandle find_material(std::string_view name) const {
        std::lock_guard lock(mutex_);
        auto it = material_name_to_id_.find(std::string(name));
        if (it == material_name_to_id_.end()) return TextMaterialHandle{0};
        return TextMaterialHandle{it->second};
    }

    // フォールバックフォントの設定
    void set_fallback(FontHandle font, const std::vector<FontHandle>& fallbacks) {
        std::lock_guard lock(mutex_);
        auto it = fonts_.find(font.id);
        if (it != fonts_.end()) {
            it->second.fallback_fonts = fallbacks;
        }
    }

    // コードポイントに対応するグリフを検索 (フォールバックチェーン込み)
    struct GlyphLookupResult {
        const Glyph* glyph = nullptr;
        FontHandle source_font{0};
    };

    GlyphLookupResult lookup_glyph(FontHandle font, uint32_t codepoint) const {
        std::lock_guard lock(mutex_);
        return lookup_glyph_unlocked(font, codepoint);
    }

private:
    GlyphLookupResult lookup_glyph_unlocked(FontHandle font, uint32_t codepoint) const {
        auto it = fonts_.find(font.id);
        if (it == fonts_.end()) return {};

        const auto* glyph = it->second.find_glyph(codepoint);
        if (glyph) return {glyph, font};

        // フォールバックチェーンを検索
        for (const auto& fb : it->second.fallback_fonts) {
            auto fb_it = fonts_.find(fb.id);
            if (fb_it == fonts_.end()) continue;
            glyph = fb_it->second.find_glyph(codepoint);
            if (glyph) return {glyph, fb};
        }

        return {};
    }
};

// グローバルインスタンス
inline FontRegistry g_font_registry;
