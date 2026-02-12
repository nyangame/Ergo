#pragma once
#include <cstdint>
#include <vector>
#include "../resource/texture_handle.hpp"

// SDF/MSDFレンダリング方式
// Siv3D の FontMethod / TextMeshPro の GlyphRenderMode に相当
enum class FontRenderMode : uint32_t {
    Bitmap,    // ビットマップ: 固定サイズ、変形に弱い
    SDF,       // Signed Distance Field: 拡大縮小に強い、単色距離場
    MSDF,      // Multi-channel SDF: 角の鋭さを維持、高品質 (推奨)
    MTSDF      // Multi-channel + True SDF: 最高品質だが生成コスト高
};

// アトラスへのパッキング戦略
// TextMeshPro の AtlasPopulationMode に相当
enum class AtlasPopulationMode : uint32_t {
    Static,     // 事前に全文字をベイク (使用文字が確定している場合に最適)
    Dynamic     // 実行時に必要なグリフを逐次追加 (チャット等の予測不能テキスト用)
};

// アトラスのピクセル配置情報 (パッキング用)
struct AtlasRect {
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

// フォントアトラスページ: 1枚のテクスチャに対応
// TextMeshPro のマルチアトラス対応
struct FontAtlasPage {
    TextureHandle texture;
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<uint8_t> pixel_data;  // CPU側のピクセルバッファ (アップロード前)
    bool dirty = false;                // GPU側テクスチャとの同期が必要か
};

// フォントアトラス: 複数ページにまたがるグリフテクスチャの管理
// Siv3D の FontAtlas / TextMeshPro の TMP_FontAsset.atlasTextures に相当
struct FontAtlas {
    FontRenderMode render_mode = FontRenderMode::MSDF;
    AtlasPopulationMode population_mode = AtlasPopulationMode::Dynamic;
    uint32_t atlas_width = 512;       // デフォルトアトラスサイズ
    uint32_t atlas_height = 512;
    uint32_t padding = 4;             // グリフ間のパディング (SDF勾配のためのスペース)
    float sdf_pixel_range = 4.0f;     // SDF/MSDFの距離レンジ (ピクセル単位)
    std::vector<FontAtlasPage> pages;

    // 現在のパッキング位置 (Simple Shelf Packing)
    uint32_t cursor_x = 0;
    uint32_t cursor_y = 0;
    uint32_t row_height = 0;
    uint32_t current_page = 0;
};
