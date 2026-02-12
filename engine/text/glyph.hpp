#pragma once
#include <cstdint>
#include "../math/vec2.hpp"
#include "../resource/texture_handle.hpp"

// グリフメトリクス: 1文字分のフォント計測情報
// Siv3D の GlyphInfo / TextMeshPro の TMP_Glyph に相当
struct GlyphMetrics {
    float width = 0.0f;       // グリフのピクセル幅
    float height = 0.0f;      // グリフのピクセル高さ
    float bearing_x = 0.0f;   // ベースラインからの水平オフセット
    float bearing_y = 0.0f;   // ベースラインからの垂直オフセット (上方が正)
    float advance = 0.0f;     // 次のグリフまでの水平距離
};

// アトラス上のグリフ位置情報 (UV矩形)
struct GlyphAtlasRegion {
    float u0 = 0.0f;         // テクスチャ左端 [0,1]
    float v0 = 0.0f;         // テクスチャ上端 [0,1]
    float u1 = 0.0f;         // テクスチャ右端 [0,1]
    float v1 = 0.0f;         // テクスチャ下端 [0,1]
    uint32_t atlas_index = 0; // マルチアトラス時のページ番号
};

// グリフ: メトリクスとアトラス上の位置をまとめた1文字の描画情報
// TextMeshPro の Glyph + GlyphRect に相当
struct Glyph {
    uint32_t codepoint = 0;   // Unicode コードポイント
    uint32_t glyph_index = 0; // フォント内グリフインデックス
    GlyphMetrics metrics;
    GlyphAtlasRegion atlas;
    float scale = 1.0f;       // MSDF 基本サイズからのスケール係数
};

// カーニングペア: 特定の文字の組み合わせにおける間隔調整
// TextMeshPro の GlyphPairAdjustmentRecord に相当
struct KerningPair {
    uint32_t first = 0;       // 先行文字のコードポイント
    uint32_t second = 0;      // 後続文字のコードポイント
    float x_advance = 0.0f;   // 水平方向の間隔調整量
};
